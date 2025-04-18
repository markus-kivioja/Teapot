#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <d3d11.h>
#include <d3dx10math.h>
#include <vector>
#include <stdint.h>

#define BINARY_MESH

class Shader;

class Object
{
public:
	Object(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename, 
		   CHAR* textureFilename, CHAR* normalFilename, CHAR* displacementFilename,
		   CHAR* specularFilename, bool isIndexed, bool isQuadData);
	~Object();

	void render(Shader* shader);

	int getIndexCount();

	void setPosition(float x, float y, float z);
	void setRotation(float rotX, float rotY, float rotZ);
	void setScale(float scale);

	void setShininess(float shininess) { m_shininess = shininess; };
	float getShininess() { return m_shininess; };

	void getTransform(D3DXMATRIX* output); 

	void calculateBoundingSphere();

	void setBackfaceCulling(bool backfaceCulling) { m_backfaceCulling = backfaceCulling; };

	bool cullBackface() { return m_backfaceCulling; };

private:
	struct Vertex
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texCoord;
		D3DXVECTOR3 normal;
		D3DXVECTOR4 tangent;
	};
	struct CacheEntry
	{
		UINT index;
		CacheEntry* pNext;
	};

	float m_x;
	float m_y;
	float m_z;
	float m_rotX;
	float m_rotY;
	float m_rotZ;
	float m_scale;

	ID3D11ShaderResourceView* m_texture;
	ID3D11ShaderResourceView* m_normalMap;
	ID3D11ShaderResourceView* m_displacementMap;
	ID3D11ShaderResourceView* m_specularMap;

	std::vector<Vertex>      m_vertices;
	std::vector<uint16_t>    m_indices;

	std::vector<CacheEntry*> m_vertexCache; 

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

	ID3D11Device*		 m_device;
	ID3D11DeviceContext* m_deviceContext;

	bool				m_backfaceCulling;

	float m_boundingSphere[4];

	float m_shininess;

	void initializeBuffers();
	void loadMesh(char*);
	void loadIndexedMesh(char*, bool);
	uint16_t addVertex(uint32_t hash, Vertex* pVertex);

	void writeMeshToFile(char* filename);
	void loadBinaryMesh(char* filename);

	struct Triangle
	{
		uint16_t indices[3];
	};

	void calculateTangents(const std::vector<Triangle>& triangles);
};

#endif