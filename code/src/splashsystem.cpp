#include "../include/splashsystem.h"
#include "../include/shaderrenderers/particleshader.h"
#include "../include/splashdrop.h"
#include "../include/constant_parameters.h"
#include <d3d11.h>
#include <cstdlib>
#include <stdint.h>

SplashSystem::SplashSystem(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ParticleShader* rainShader) :
	m_device(device),
	m_deviceContext(deviceContext),
	m_instanceBuffer(0),
	m_vertexBuffer(0),
	m_indexBuffer(0),
	m_shader(rainShader)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, instanceBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;

	VertexType* vertices = new VertexType[4];
	uint16_t* indices = new uint16_t[6];

	vertices[0].position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].position = D3DXVECTOR3(0.0f, SPLASH_PARTICLE_RADIUS, 0.0f);
	vertices[1].texture = D3DXVECTOR2(0.0f, 1.0f);
	vertices[2].position = D3DXVECTOR3(SPLASH_PARTICLE_RADIUS, SPLASH_PARTICLE_RADIUS, 0.0f);
	vertices[2].texture = D3DXVECTOR2(1.0f, 1.0f);
	vertices[3].position = D3DXVECTOR3(SPLASH_PARTICLE_RADIUS, 0.0f, 0.0f);
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

    vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

    device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

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
}

SplashSystem::~SplashSystem()
{
	int dropsNum = m_drops.size();
	for (int i = 0; i < dropsNum; ++i)
	{
		delete m_drops[i];
	}
	m_drops.clear();
	if (m_indexBuffer)
		m_indexBuffer->Release();
	if (m_vertexBuffer)
		m_vertexBuffer->Release();
	if (m_instanceBuffer)
		m_instanceBuffer->Release();
	m_shader = 0;
	m_indexBuffer = 0;
	m_vertexBuffer = 0;
}

void SplashSystem::render(D3DXVECTOR3 cameraDirection)
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
	D3DXMatrixRotationYawPitchRoll(&transform, atan2(x, z), -atan2(y, sqrt(x*x + z*z)), 0.0f);

	m_shader->setWorldMatrix(transform);

	m_shader->setTechnique(SPLASH);
	m_shader->render(6, dropsNum);
}

void SplashSystem::update(int dt)
{
	for (unsigned int i = 0; i < m_drops.size(); ++i)
	{
		m_drops[i]->update(dt);
		if (m_drops[i]->isDead())
		{
			delete m_drops[i];
			m_drops.erase(m_drops.begin() + i);
			i--;
		}
	}
}

void SplashSystem::splash(D3DXVECTOR3 pos, D3DXVECTOR3 dir)
{
	for (int i = 0; i < 10; ++i)
	{
		D3DXVECTOR3 randDir;
		randDir.x = (rand() % 10) / 30.0f - 0.25f;
		randDir.y = (rand() % 10) / 30.0f - 0.25f;
		randDir.z = (rand() % 10) / 30.0f - 0.25f;
		m_drops.push_back(new SplashDrop(m_shader, pos, (dir + randDir)*SPLASH_PARTICLE_SPEED,
			SPLASH_PARTICLE_LIFESPAN, SPLASH_PARTICLE_GRAVITY));
	}
}