#include "../include/raincaster.h"
#include "../include/shaderrenderers/particleshader.h"
#include "../include/shaderrenderers/gbuffershader.h"
#include "../include/raindrop.h"
#include "../include/splashsystem.h"
#include "../include/constant_parameters.h"
#include "../include/funcs.h"
#include <d3d11.h>
#include <stdint.h>

RainCaster::RainCaster(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
					   D3DXVECTOR4* nData, GBufferShader* gBufShader, ParticleShader* rainShader) :
	m_normalBufferData(nData),
	m_device(device),
	m_deviceContext(deviceContext),
	m_gBufferShader(gBufShader),
	m_indexBuffer(0),
	m_vertexBuffer(0),
	m_instanceBuffer(0),
	m_splashSystem(0),
	m_shader(rainShader)
{

}

RainCaster::~RainCaster()
{
	int dropsNum = m_drops.size();
	for (int i = 0; i < dropsNum; ++i)
		delete m_drops[i];
	m_drops.clear();
	delete[] m_normalBufferData;
	m_normalBufferData = 0;
	delete m_shader;
	m_shader = 0;
	if (m_indexBuffer)
		m_indexBuffer->Release();
	m_indexBuffer = 0;
	if (m_vertexBuffer)
		m_vertexBuffer->Release();
	m_vertexBuffer = 0;
	if (m_instanceBuffer)
		m_instanceBuffer->Release();
	m_instanceBuffer = 0;
	if (m_splashSystem)
		delete m_splashSystem;
	m_splashSystem = 0;
}

bool RainCaster::init()
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, instanceBufferDesc;

	VertexType* vertices = new VertexType[4];
	uint16_t* indices = new uint16_t[6];

	float height = 1.0f;
	vertices[0].position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].position = D3DXVECTOR3(0.0f, RAIN_DROP_HEIGHT, 0.0f);
	vertices[1].texture = D3DXVECTOR2(0.0f, 1.0f);
	vertices[2].position = D3DXVECTOR3(RAIN_DROP_WIDTH, RAIN_DROP_HEIGHT, 0.0f);
	vertices[2].texture = D3DXVECTOR2(1.0f, 1.0f);
	vertices[3].position = D3DXVECTOR3(RAIN_DROP_WIDTH, 0.0f, 0.0f);
	vertices[3].texture = D3DXVECTOR2(1.0f, 0.0f);
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * 4;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData, indexData;
    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

    m_device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(uint16_t) * 6;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	m_device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);

	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.ByteWidth = sizeof(InstanceType) * 1000;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&instanceBufferDesc, nullptr, &m_instanceBuffer);

	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	m_splashSystem = new SplashSystem(m_device, m_deviceContext, m_shader);

	return true;
}

void RainCaster::render(D3DXVECTOR3 cameraDirection)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	InstanceType* instanceData = (InstanceType*)(mappedResource.pData);

	int dropsNum = m_drops.size();
	for (int i = 0; i < dropsNum; ++i)
		m_drops[i]->getPosition((float*)(&instanceData[i]));

	m_deviceContext->Unmap(m_instanceBuffer, 0);

	unsigned int strides[] = {sizeof(VertexType), sizeof(InstanceType)}; 
	unsigned int offsets[] = {0, 0};
	ID3D11Buffer* vertexBuffers[] = {m_vertexBuffer, m_instanceBuffer};

	m_deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
	m_deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXMATRIX transform;
	float x = cameraDirection.x, y = cameraDirection.y, z = cameraDirection.z;
	D3DXMatrixRotationYawPitchRoll(&transform, atan2(x, z), 0.0f, 0.0f);

	m_shader->setWorldMatrix(transform);

	m_shader->setTechnique(DROP);
	m_shader->render(6, dropsNum);

	m_splashSystem->render(cameraDirection);
}

void RainCaster::update(int dt)
{
	for (int i = 0; i < RAIN_RATE*dt/TARGET_FRAME_TIME; ++i)
	{
		float x = (rand() % 100) / 10.0f - 5.0f;
		float z = (rand() % 100) / 10.0f - 5.0f;

		int v = (int)((x + 5.0f) / 10.0f * BUFS_WIDTH);
		int u = (int)((z + 5.0f) / 10.0f * BUFS_HEIGHT);

		D3DXVECTOR4 gBufData =  m_normalBufferData[BUFS_WIDTH*v+u];
		
		float dieHeight = 5.0f - gBufData.x + 1.0f;

		m_drops.push_back(new RainDrop(m_shader, x, 6.0f, z, RAIN_SPEED, dieHeight, D3DXVECTOR3(gBufData.y, gBufData.z, gBufData.w)));
	}

	for (unsigned int i = 0; i < m_drops.size(); ++i)
	{
		m_drops[i]->update(dt);
		if (m_drops[i]->isDead())
		{
			D3DXVECTOR3 splashPos;
			m_drops[i]->getDiePosition(splashPos);
			D3DXVECTOR3 normal;
			m_drops[i]->getDiePosNormal(normal);
			D3DXVECTOR3 dropDir(0.0f, 1.0f, 0.0f);
			D3DXVECTOR3 splashDir = normalize(2*dot(dropDir, normal)*normal - dropDir);
			m_splashSystem->splash(splashPos, splashDir);
			D3DXVECTOR3 dir(0.0f, 0.0f, 0.0f);
			if (splashPos.y > 1.01f)
			{
				float dx = normal.x*GBUF_LOOKUP_DISTANCE_FACTOR;
				float dz = normal.z*GBUF_LOOKUP_DISTANCE_FACTOR;
				int v = (int)((splashPos.x + dx + 5.0f) / 10.0f * BUFS_WIDTH);
				int u = (int)((splashPos.z + dz + 5.0f) / 10.0f * BUFS_HEIGHT);
				float dy = 5.0f - m_normalBufferData[BUFS_WIDTH*v+u].x + 1.0f - splashPos.y;
				dir = D3DXVECTOR3(dx, dy, dz);
				m_gBufferShader->addDribbleEffect(splashPos, dir);
			}
			else
				m_gBufferShader->addWaveEffect(splashPos);
			delete m_drops[i];
			m_drops.erase(m_drops.begin() + i);
			i--;
		}
	}

	m_splashSystem->update(dt);
}