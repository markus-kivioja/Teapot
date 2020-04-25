#include "../include/lightmanager.h"
#include "../include/shaderrenderers/particleshader.h"
#include "../include/funcs.h"
#include <stdint.h>

LightManager::LightManager(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ParticleShader* particleShader) :
	m_particleShader(particleShader),
	m_device(device),
	m_deviceContext(deviceContext)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, instanceBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;

	VertexType* vertices = new VertexType[4];
	uint16_t* indices = new uint16_t[6];

	vertices[0].position = D3DXVECTOR3(-LIGHT_PARTICLE_RADIUS, -LIGHT_PARTICLE_RADIUS, 0.0f);
	vertices[0].texture = D3DXVECTOR2(0.0f, 0.0f);
	vertices[1].position = D3DXVECTOR3(-LIGHT_PARTICLE_RADIUS, LIGHT_PARTICLE_RADIUS, 0.0f);
	vertices[1].texture = D3DXVECTOR2(0.0f, 1.0f);
	vertices[2].position = D3DXVECTOR3(LIGHT_PARTICLE_RADIUS, LIGHT_PARTICLE_RADIUS, 0.0f);
	vertices[2].texture = D3DXVECTOR2(1.0f, 1.0f);
	vertices[3].position = D3DXVECTOR3(LIGHT_PARTICLE_RADIUS, -LIGHT_PARTICLE_RADIUS, 0.0f);
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
    instanceBufferDesc.ByteWidth = sizeof(InstanceType) * (POINT_LIGHT_COUNT + SPOT_LIGHT_COUNT);
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	m_device->CreateBuffer(&instanceBufferDesc, nullptr, &m_instanceBuffer);

	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

	D3DXVECTOR4 colorPalette[1000];
	for (int i = 0; i < 1000; ++i)
		colorPalette[i] = D3DXVECTOR4(random(0, 1.2f), random(0, 1.2f), random(0, 1.2f), 1.0f);

	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		float x = random(-5.0f, 5.0f);
		float z = random(-5.0f, 5.0f);
		m_pointLights[i].position = D3DXVECTOR4(x, 2.05f, z, POINT_LIGHT_RADIUS);
		m_pointLights[i].color = colorPalette[rand() % 1000];
	}

	float angle = (float)D3DX_PI / 6.0f;
	float sinAngle = sin(angle);
	m_spotLights[0].positionSin = D3DXVECTOR4(-4.0f, 2.0f, -1.0f, sinAngle);
	m_spotLights[0].directionCos = D3DXVECTOR4(normalize(D3DXVECTOR3(4.0f, -1.0f, 4.0f)), cos(angle));
	m_spotLights[0].colorRcpSin = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f/sinAngle);

	m_spotLights[1].positionSin = D3DXVECTOR4(4.0f, 2.0f, 4.0f, sinAngle);
	m_spotLights[1].directionCos = D3DXVECTOR4(normalize(D3DXVECTOR3(-4.0f, -1.0f, -4.0f)), cos(angle));
	m_spotLights[1].colorRcpSin = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f/sinAngle);

	m_spotLights[2].positionSin = D3DXVECTOR4(-4.0f, 4.0f, 1.0f, sinAngle);
	m_spotLights[2].directionCos = D3DXVECTOR4(normalize(D3DXVECTOR3(4.0f, -3.0f, -4.0f)), cos(angle));
	m_spotLights[2].colorRcpSin = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f/sinAngle);

	m_spotLights[3].positionSin = D3DXVECTOR4(4.0f, 2.0f, -4.0f, sinAngle);
	m_spotLights[3].directionCos = D3DXVECTOR4(normalize(D3DXVECTOR3(-4.0f, -1.0f, 4.0f)), cos(angle));
	m_spotLights[3].colorRcpSin = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f/sinAngle);
}

LightManager::~LightManager()
{
	if (m_indexBuffer)
		m_indexBuffer->Release();
	if (m_instanceBuffer)
		m_instanceBuffer->Release();
	if (m_vertexBuffer)
		m_vertexBuffer->Release();
	m_indexBuffer = 0;
	m_instanceBuffer = 0;
	m_vertexBuffer = 0;
}

float time = 0.0f;
void LightManager::update(int dt)
{
	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
		m_pointLights[i].position.y = 1.05f + (sin(time + m_pointLights[i].position.x * m_pointLights[i].position.z) + 1.0f) * 0.8f;

	m_spotLights[0].positionSin.y = 4.0f + sin(time);
	m_spotLights[0].directionCos = D3DXVECTOR4(normalize(D3DXVECTOR3(4.0f, 1.0f - m_spotLights[0].positionSin.y,4.0f)), m_spotLights[0].directionCos.w);

	m_spotLights[1].directionCos.x = cos(time);
	m_spotLights[1].directionCos.z = sin(time);

	m_spotLights[2].directionCos.x = cos(time);
	m_spotLights[2].directionCos.z = sin(time);

	time += 0.05f;
}

void LightManager::render(D3DXVECTOR3 cameraDirection)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	InstanceType* instanceData = (InstanceType*)(mappedResource.pData);

	for (int i = 0; i < POINT_LIGHT_COUNT; ++i)
	{
		instanceData[i].instancePos = D3DXVECTOR3(m_pointLights[i].position.x, m_pointLights[i].position.y, m_pointLights[i].position.z);
		instanceData[i].instanceColor = D3DXVECTOR3(m_pointLights[i].color.x, m_pointLights[i].color.y, m_pointLights[i].color.z);
	}
	for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
	{
		instanceData[POINT_LIGHT_COUNT + i].instancePos = D3DXVECTOR3(m_spotLights[i].positionSin.x, m_spotLights[i].positionSin.y, m_spotLights[i].positionSin.z);
		instanceData[POINT_LIGHT_COUNT + i].instanceColor = D3DXVECTOR3(m_spotLights[i].colorRcpSin.x, m_spotLights[i].colorRcpSin.y, m_spotLights[i].colorRcpSin.z);
	}

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

	m_particleShader->setWorldMatrix(transform);

	m_particleShader->setTechnique(POINT_LIGHT);
	m_particleShader->render(6, POINT_LIGHT_COUNT + SPOT_LIGHT_COUNT);
}