#include "../../include/shaderrenderers/lightingshader.h"
#include "../../include/lightmanager.h"
#include "../../include/constant_parameters.h"
#include "../../include/funcs.h"
#include "../../include/camera.h"
#include "../../include/light.h"
#include <d3dx11async.h>

#define THREADS_PER_GROUP_X 16
#define THREADS_PER_GROUP_Y 16

LightingShader::LightingShader(Camera* camera, const PointLight* pointLightBuffer, const SpotLight* spotLightBuffer, int width, int height, float nearPlane, float farPlane) :
	Shader(),
	m_camera(camera),
	m_computeShader(0),
	m_lightTransformShader(0),
	m_matrixBuffer(0),
	m_pointLightBuffer(pointLightBuffer),
	m_spotLightBuffer(spotLightBuffer),
	m_width(width),
	m_height(height),
	m_vsPointLightBuffer(0),
	m_vsSpotLightBuffer(0),
	m_vsPointLightSRV(0),
	m_vsSpotLightSRV(0),
	m_vsPointLightUAV(0),
	m_vsSpotLightUAV(0),
	m_nearPlane(nearPlane),
	m_farPlane(farPlane)
{
}

LightingShader::~LightingShader()
{
	if (m_matrixBuffer)
		m_matrixBuffer->Release();
	m_matrixBuffer = 0;
	if (m_computeShader)
		m_computeShader->Release();
	m_computeShader = 0;
	if (m_vsPointLightBuffer)
		m_vsPointLightBuffer->Release();
	m_vsPointLightBuffer = 0;
	if (m_vsPointLightSRV)
		m_vsPointLightSRV->Release();
	m_vsPointLightSRV = 0;
	if (m_vsPointLightUAV)
		m_vsPointLightUAV->Release();
	m_vsPointLightUAV = 0;
	if (m_vsSpotLightBuffer)
		m_vsSpotLightBuffer->Release();
	m_vsSpotLightBuffer = 0;
	if (m_vsSpotLightSRV)
		m_vsSpotLightSRV->Release();
	m_vsSpotLightSRV = 0;
	if (m_vsSpotLightUAV)
		m_vsSpotLightUAV->Release();
	m_vsSpotLightUAV = 0;
	if (m_lightTransformShader)
		m_lightTransformShader->Release();
	m_lightTransformShader = 0;
}

bool LightingShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	ID3D10Blob* computeShaderBuffer = 0;
	ID3D10Blob* errorMessage = 0;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* lightShaderFile = "shaders/lighting.sb";

	vs_stream.open(lightShaderFile, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreateComputeShader(vs_data, vs_size, NULL, &m_computeShader);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", lightShaderFile);
		return false;
	}

	char* lightTransformShaderFile = "shaders/lighttransform.sb";

	vs_stream.open(lightTransformShaderFile, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreateComputeShader(vs_data, vs_size, NULL, &m_lightTransformShader);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", lightShaderFile);
		return false;
	}

	D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	D3D11_BUFFER_DESC viewMatrixBufferDesc;
	viewMatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	viewMatrixBufferDesc.ByteWidth = sizeof(D3DXMATRIX);
	viewMatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	viewMatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	viewMatrixBufferDesc.MiscFlags = 0;
	viewMatrixBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&viewMatrixBufferDesc, NULL, &m_viewMatrixBuffer);

	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	lightBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS; 
	lightBufferDesc.CPUAccessFlags = 0; 
	lightBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; 
	lightBufferDesc.StructureByteStride = sizeof(PointLight); 
	lightBufferDesc.ByteWidth = sizeof(PointLight) * POINT_LIGHT_COUNT;  
	device->CreateBuffer(&lightBufferDesc, 0, &m_vsPointLightBuffer);
	device->CreateShaderResourceView(m_vsPointLightBuffer, NULL, &m_vsPointLightSRV);
	device->CreateUnorderedAccessView(m_vsPointLightBuffer, NULL, &m_vsPointLightUAV);

	lightBufferDesc.StructureByteStride = sizeof(SpotLight); 
	lightBufferDesc.ByteWidth = sizeof(SpotLight) * SPOT_LIGHT_COUNT;
	device->CreateBuffer(&lightBufferDesc, 0, &m_vsSpotLightBuffer);
	device->CreateShaderResourceView(m_vsSpotLightBuffer, NULL, &m_vsSpotLightSRV);
	device->CreateUnorderedAccessView(m_vsSpotLightBuffer, NULL, &m_vsSpotLightUAV);

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; 
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; 
	lightBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED; 
	lightBufferDesc.StructureByteStride = sizeof(PointLight); 
	lightBufferDesc.ByteWidth = sizeof(PointLight) * POINT_LIGHT_COUNT;
	device->CreateBuffer(&lightBufferDesc, 0, &m_wsPointLightBuffer);
	device->CreateShaderResourceView(m_wsPointLightBuffer, NULL, &m_wsPointLightSRV);

	lightBufferDesc.StructureByteStride = sizeof(SpotLight); 
	lightBufferDesc.ByteWidth = sizeof(SpotLight) * SPOT_LIGHT_COUNT;
	device->CreateBuffer(&lightBufferDesc, 0, &m_wsSpotLightBuffer);
	device->CreateShaderResourceView(m_wsSpotLightBuffer, NULL, &m_wsSpotLightSRV);

	D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
#ifdef REVERSE_DEPTH
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_GREATER;
#else
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
#endif
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &m_shadowSampler);

	return true;
}

void LightingShader::transformLights()
{
	m_deviceContext->CSSetShader(m_lightTransformShader, NULL, 0);

	D3DXMATRIX cameraViewMatrix;
	m_camera->getViewMatrix(cameraViewMatrix);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_viewMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	D3DXMATRIX* matrixData = (D3DXMATRIX*)mappedResource.pData;
	D3DXMatrixTranspose(matrixData, &cameraViewMatrix);
    m_deviceContext->Unmap(m_viewMatrixBuffer, 0);
	m_deviceContext->CSSetConstantBuffers(0, 1, &m_viewMatrixBuffer);

	m_deviceContext->Map(m_wsPointLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	PointLight* pointLightData = (PointLight*)mappedResource.pData;
	memcpy(pointLightData, m_pointLightBuffer, sizeof(PointLight) * POINT_LIGHT_COUNT);
	m_deviceContext->Unmap(m_wsPointLightBuffer, 0);

	m_deviceContext->Map(m_wsSpotLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	SpotLight* spotLightData = (SpotLight*)mappedResource.pData;
	memcpy(spotLightData, m_spotLightBuffer, sizeof(SpotLight) * SPOT_LIGHT_COUNT);
	m_deviceContext->Unmap(m_wsSpotLightBuffer, 0);

	ID3D11ShaderResourceView* srvs[] = {m_wsPointLightSRV, m_wsSpotLightSRV};
	m_deviceContext->CSSetShaderResources(0, 2, srvs);
	ID3D11UnorderedAccessView* uavs[] = {m_vsPointLightUAV, m_vsSpotLightUAV};
	m_deviceContext->CSSetUnorderedAccessViews(0, 2, uavs, 0);
	m_deviceContext->Dispatch((POINT_LIGHT_COUNT + SPOT_LIGHT_COUNT + 64 - 1) / 64, 1, 1);
}

void LightingShader::render(int indexCount, int instanceCount)
{
    m_deviceContext->CSSetShader(m_computeShader, NULL, 0);

	D3DXMATRIX projection;
	m_camera->getProjectionMatrix(projection);
	D3DXMATRIX inverseView;
	m_camera->getViewMatrix(inverseView);
	D3DXMatrixInverse(&inverseView, NULL, &inverseView);

	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 position;
	D3DXVECTOR3 direction;
	D3DXVECTOR3 lookAt;
	D3DXMATRIX lightView;
	D3DXMATRIX lightProjection;
	D3DXMATRIX inverseViewLightVP;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBuffer* matrixData = (MatrixBuffer*)mappedResource.pData;
	D3DXMatrixTranspose(&(matrixData->cameraProjection), &projection);
	D3DXMatrixTranspose(&(matrixData->inverseView), &inverseView);
	for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
	{
		position = D3DXVECTOR3(m_spotLightBuffer[i].positionSin.x, m_spotLightBuffer[i].positionSin.y, m_spotLightBuffer[i].positionSin.z);
		direction = D3DXVECTOR3(m_spotLightBuffer[i].directionCos.x, m_spotLightBuffer[i].directionCos.y, m_spotLightBuffer[i].directionCos.z);
		lookAt = position + direction;
		D3DXMatrixLookAtLH(&lightView, &position, &lookAt, &up);
		D3DXMatrixPerspectiveFovLH(&lightProjection, asin(m_spotLightBuffer[i].positionSin.w) * 2, 1.0f, m_nearPlane, m_farPlane);

		D3DXMatrixMultiply(&inverseViewLightVP, &inverseView, &lightView);
		D3DXMatrixMultiply(&inverseViewLightVP, &inverseViewLightVP, &lightProjection);

		D3DXMatrixTranspose(&(matrixData->inverseViewLightVP[i]), &inverseViewLightVP);
	}
	m_deviceContext->Unmap(m_matrixBuffer, 0);
	m_deviceContext->CSSetConstantBuffers(0, 1, &m_matrixBuffer);

	ID3D11ShaderResourceView* srvs[] = {m_vsPointLightSRV, m_vsSpotLightSRV};
	m_deviceContext->CSSetShaderResources(4, 2, srvs);

	m_deviceContext->CSSetSamplers(1, 1, &m_shadowSampler);

	m_deviceContext->Dispatch((m_width + THREADS_PER_GROUP_X + 1) / THREADS_PER_GROUP_X, (m_height + THREADS_PER_GROUP_Y + 1) / THREADS_PER_GROUP_Y, 1);
}