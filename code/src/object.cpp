#include "object.h"

#include "shaderrenderers/shader.h"
#include "funcs.h"
#include "Miniball.h"

#include "d3dx11.h"
#include <fstream>

Object::Object(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename, CHAR* textureFilename,  
			   CHAR* normalFilename, CHAR* displacementFilename, CHAR* specularFilename, bool isIndexed, bool isQuadData) :
	m_vertexBuffer(0),
	m_indexBuffer(0),
	m_texture(0),
	m_x(0.0f),
	m_y(0.0f),
	m_z(0.0f),
	m_rotX(0.0f),
	m_rotY(0.0f),
	m_rotZ(0.0f),
	m_scale(1.0f),
	m_device(device),
	m_deviceContext(deviceContext),
	m_backfaceCulling(true)
{
#ifdef BINARY_MESH
	loadBinaryMesh(modelFilename);
#else
	if (isIndexed)
		loadIndexedMesh(modelFilename, isQuadData);
	else
		loadMesh(modelFilename);

	writeMeshToFile(modelFilename);
#endif

	initializeBuffers();

	HRESULT result;

	D3DX11CreateShaderResourceViewFromFile(device, textureFilename, NULL, NULL, &m_texture, &result);
	if(FAILED(result))
		output("LOADING COLOR FOR %s FAILED\n", modelFilename);
	D3DX11CreateShaderResourceViewFromFile(device, normalFilename, NULL, NULL, &m_normalMap, &result);
	if(FAILED(result))
		output("LOADING NORMAL FOR %s FAILED\n", modelFilename);
	D3DX11CreateShaderResourceViewFromFile(device, displacementFilename, NULL, NULL, &m_displacementMap, &result);
	if(FAILED(result))
		output("LOADING DISPLACEMENT FOR %s FAILED\n", modelFilename);
	D3DX11CreateShaderResourceViewFromFile(device, specularFilename, NULL, NULL, &m_specularMap, &result);
	if(FAILED(result))
		output("LOADING SPECULAR FOR %s FAILED\n", modelFilename);

	//calculateBoundingSphere();
	//output("%s BOUNDING SPHERE: %f, %f, %f, %f\n", modelFilename, m_boundingSphere[0], m_boundingSphere[1], m_boundingSphere[2], m_boundingSphere[3]);

	//m_normalMap = 0;
	//m_displacementMap = 0;
	//m_specularMap = 0;
}

Object::~Object()
{
	m_device->Release();
	m_deviceContext->Release();
	if (m_texture)
		m_texture->Release();
	m_texture = 0;
	if (m_normalMap)
		m_normalMap->Release();
	m_normalMap = 0;
	if (m_displacementMap)
		m_displacementMap->Release();
	m_displacementMap = 0;
	if (m_specularMap)
		m_specularMap->Release();
	m_specularMap = 0;
	if (m_indexBuffer)
		m_indexBuffer->Release();
	m_indexBuffer = 0;
	if (m_vertexBuffer)
		m_vertexBuffer->Release();
	m_vertexBuffer = 0;
}

void Object::getTransform(D3DXMATRIX* output)
{
	D3DXMatrixScaling(output, m_scale, m_scale, m_scale);

	D3DXMATRIX rotation;
	D3DXMatrixRotationYawPitchRoll(&rotation, m_rotY, m_rotX, m_rotZ);
	
	D3DXMatrixMultiply(output, output, &rotation);

	D3DXMATRIX translation;
	D3DXMatrixTranslation(&translation, m_x, m_y, m_z);

	D3DXMatrixMultiply(output, output, &translation);
}

void Object::render(Shader* shader)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(Vertex); 
	offset = 0;

	ID3D11ShaderResourceView* textures[] = {m_texture, m_normalMap, m_displacementMap, m_specularMap};
	m_deviceContext->PSSetShaderResources(0, 4, textures);

	m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMATRIX worldMatrix;
	getTransform(&worldMatrix);

	shader->setWorldMatrix(worldMatrix);

	shader->render(m_indices.size(), 1);
}

void Object::initializeBuffers()
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * m_vertices.size();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

    vertexData.pSysMem = &m_vertices[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

    m_device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(uint16_t) * m_indices.size();
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

    indexData.pSysMem = &m_indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
}

uint16_t Object::addVertex(uint32_t hash, Vertex* pVertex)
{
    bool bFoundInList = false;
    uint16_t index = 0;

    if ((uint32_t)m_vertexCache.size() > hash)
    {
        CacheEntry* pEntry = m_vertexCache[hash];
        while ( pEntry != NULL )
        {
            Vertex* pCacheVertex = &(m_vertices[pEntry->index]);

            if (0 == memcmp(pVertex, pCacheVertex, sizeof(Vertex)))
            {
                bFoundInList = true;
				pCacheVertex->normal += pVertex->normal;
                index = pEntry->index;
                break;
            }

            pEntry = pEntry->pNext;
        }
    }

    if( !bFoundInList )
    {
        index = m_vertices.size();
        m_vertices.push_back( *pVertex );

        CacheEntry* pNewEntry = new CacheEntry;
        if (pNewEntry == NULL)
            return (uint16_t)-1;

        pNewEntry->index = index;
        pNewEntry->pNext = NULL;

        while ((uint32_t)m_vertexCache.size() <= hash)
        {
            m_vertexCache.push_back(nullptr);
        }

        CacheEntry* pCurEntry = m_vertexCache[hash];
        if (pCurEntry == NULL)
        {
            m_vertexCache[hash] = pNewEntry;
        }
        else
        {
            while (pCurEntry->pNext != NULL)
            {
                pCurEntry = pCurEntry->pNext;
            }

            pCurEntry->pNext = pNewEntry;
        }
    }

    return index;
}

void Object::calculateTangents(const std::vector<Triangle>& triangles)
{
	uint32_t vertexCount = m_vertices.size();
	D3DXVECTOR3 *tan1 = new D3DXVECTOR3[vertexCount * 2];
	D3DXVECTOR3 *tan2 = tan1 + vertexCount;
	memset(tan1, 0, vertexCount * sizeof(D3DXVECTOR3) * 2);
	
	uint32_t triangleCount = triangles.size();
	for (uint32_t i = 0; i < triangleCount; i++)
	{
		uint16_t i1 = triangles[i].indices[0];
		uint16_t i2 = triangles[i].indices[1];
		uint16_t i3 = triangles[i].indices[2];
       
		const D3DXVECTOR3& v1 = m_vertices[i1].position;
		const D3DXVECTOR3& v2 = m_vertices[i2].position;
		const D3DXVECTOR3& v3 = m_vertices[i3].position;
       
		const D3DXVECTOR2& w1 = m_vertices[i1].texCoord;
		const D3DXVECTOR2& w2 = m_vertices[i2].texCoord;
		const D3DXVECTOR2& w3 = m_vertices[i3].texCoord;
       
		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;
       
		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;
       
		float r = 1.0F / (s1 * t2 - s2 * t1);
		D3DXVECTOR3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
		D3DXVECTOR3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
       
		tan1[i1] += sdir;
		tan1[i2] += sdir;
		tan1[i3] += sdir;
       
		tan2[i1] += tdir;
		tan2[i2] += tdir;
		tan2[i3] += tdir;
	}
   
	for (uint32_t a = 0; a < vertexCount; a++)
	{
		const D3DXVECTOR3& n = m_vertices[a].normal;
		const D3DXVECTOR3& t = tan1[a];

		// Orthogonalize
		m_vertices[a].tangent = D3DXVECTOR4(normalize(t - n * dot(n, t)), 0.0f);
		m_vertices[a].tangent.w = (dot(cross(n, t), tan2[a]) < 0.0F) ? -1.0F : 1.0F;
	}
   
	delete[] tan1;
}

enum ReadingType
{
	POSITION,
	TEXCOORD,
	NORMAL,
	FACE
};

void Object::loadIndexedMesh(char* filename, bool quadData)
{
	std::ifstream fin;
	char input;

	fin.open(filename);

	if(fin.fail())
		return;

	fin.get(input);
	while(input != 'v')
		fin.get(input);

	std::vector<D3DXVECTOR3> positions;
	std::vector<D3DXVECTOR2> texCoords;
	std::vector<D3DXVECTOR3> normals;

	float x, y, z;

	std::vector<Triangle> triangles;
	Triangle triangle;
	uint16_t index;

	ReadingType reading = POSITION;
	int indicesPerFace = quadData ? 4 : 3;
	while (fin.good())
	{
		switch (reading)
		{
			case POSITION:
				fin >> x >> y >> z;
				positions.push_back(D3DXVECTOR3(x, y, z));
				break;
			case TEXCOORD:
				fin >> x >> y;
				texCoords.push_back(D3DXVECTOR2(x, y));
				break;
			case NORMAL:
				fin >> x >> y >> z;
				normals.push_back(D3DXVECTOR3(x, y, z));
				break;
			case FACE:
				uint16_t firstPosIndex;
				Vertex firstVertex;
				for (int i = 0; i < indicesPerFace; ++i)
				{
					uint16_t posIndex, uvIndex, normalIndex;
					char slash;
					fin >> posIndex >> slash >> uvIndex >> slash >> normalIndex;
					
					Vertex vertex;
					vertex.position = positions[posIndex - 1];
					vertex.texCoord = texCoords[uvIndex - 1];
					vertex.normal = normals[normalIndex - 1];
					vertex.tangent = D3DXVECTOR4(0, 0, 0, 0);

					index = addVertex(posIndex, &vertex);
					m_indices.push_back(index);
					
					if (i < 3)
						triangle.indices[i] = index;
					else
						triangle.indices[1] = index;

					if (quadData)
					{
						if (!i)
						{
							firstPosIndex = posIndex;
							firstVertex = vertex;
						}
						else if (i == 2)
						{
							triangles.push_back(triangle);
							
							index = addVertex(posIndex, &vertex);
							m_indices.push_back(index);
							triangle.indices[0] = index;
						}
					}
				}
				if (quadData)
				{
					index = addVertex(firstPosIndex, &firstVertex);
					m_indices.push_back(index);
					triangle.indices[2] = index;
				}
				triangles.push_back(triangle);
				break;
		}

		fin.get(input);
		while (input != 'v' && input != 'f' && fin.good())
		{
			fin.get(input);
		}
		if (input == 'f')
		{
			reading = FACE;
		}
		else
		{
			fin.get(input);
			if (input == 't') reading = TEXCOORD;
			else if (input == 'n') reading = NORMAL;
			else reading = POSITION;
		}
	}

	fin.close();
	
	calculateTangents(triangles);
}

void Object::loadMesh(char* filename)
{
	std::ifstream fin;
	char input;

	fin.open(filename);

	if(fin.fail())
		return;

	fin.get(input);
	while(input != ':')
		fin.get(input);

	fin.get(input);
	while(input != ':')
		fin.get(input);
	fin.get(input);
	fin.get(input);

	std::vector<Triangle> triangles;
	Triangle triangle;

	int i = 0;
	while (fin.good())
	{
		Vertex vertex;

		fin >> vertex.position.x >> vertex.position.y >> vertex.position.z;
		fin >> vertex.texCoord.x >> vertex.texCoord.y;
		fin >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z;

		m_vertices.push_back(vertex);
		m_indices.push_back(i);
		
		triangle.indices[i % 3] = i;
		if (i % 3 == 2)
			triangles.push_back(triangle);
		
		i++;
	}

	fin.close();

	calculateTangents(triangles);
}

void Object::loadBinaryMesh(char* filename)
{
	std::ifstream file(filename, std::ios::in|std::ios::binary|std::ios::ate);
	if (file.is_open())
	{
		std::ifstream::pos_type
		size = file.tellg();
		file.seekg(0, std::ios::beg);
		
		uint32_t vertexCount = 0, indexCount = 0;
		file.read((char*)&vertexCount, 4);
		file.read((char*)&indexCount, 4);
		//file.read((char*)m_boundingSphere, 16);

		m_vertices.resize(vertexCount);
		m_indices.resize(indexCount);

		file.read((char*)&m_vertices[0], sizeof(Vertex) * vertexCount);
		file.read((char*)&m_indices[0], sizeof(uint16_t) * indexCount);

		file.close();
	}
}

void Object::writeMeshToFile(char* filename)
{
	std::string fullname = std::string(filename);
	fullname = fullname.substr(0, fullname.length() - 3).append("mymesh");
	std::ofstream file(fullname.c_str(), std::ios::out|std::ios::binary);
	if (file.is_open())
	{
		uint32_t vertexCount = m_vertices.size(), indexCount = m_indices.size();
		file.write((char*)&vertexCount, 4);
		file.write((char*)&indexCount, 4);
		file.write((char*)m_boundingSphere, 16);
		file.write((char*)&m_vertices[0], m_vertices.size()*sizeof(Vertex));
		file.write((char*)&m_indices[0], m_indices.size()*sizeof(uint16_t));
		file.close();
	}
	else
	{
		output("Couldn't open file %s\n", filename);
	}
}

void Object::setPosition(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

void Object::setRotation(float rotX, float rotY, float rotZ)
{
	m_rotX = rotX;
	m_rotY = rotY;
	m_rotZ = rotZ;
}

void Object::setScale(float scale)
{
	m_scale = scale;
}

void Object::calculateBoundingSphere()
{
	typedef float mytype;
	typedef mytype* const* PointIterator; 
	typedef const mytype* CoordIterator;

	int d = 3;
	int n = m_vertices.size();

	mytype** ap = new mytype*[n];
 	for (int i = 0; i < n; ++i)
	{
		mytype* p = new mytype[d];
		p[0] = m_vertices[i].position.x;
		p[1] = m_vertices[i].position.y;
		p[2] = m_vertices[i].position.z;
		ap[i] = p;
	}

	typedef Miniball::Miniball<Miniball::CoordAccessor<PointIterator, CoordIterator>> MB;
	MB mb(d, ap, ap+n);
	
	const mytype* center = mb.center();

	for (int i = 0; i < d; ++i)
		m_boundingSphere[i] = center[i];
	m_boundingSphere[3] = sqrt(mb.squared_radius());
}
