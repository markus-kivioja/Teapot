#include "../../include/shaderrenderers/ssaoshader.h"
#include "../../include/camera.h"
#include "../../include/funcs.h"
#include <fstream>
#include <d3dx11async.h>

#define THREADS_PER_GROUP_X 16
#define THREADS_PER_GROUP_Y 16

SSAOShader::SSAOShader(Camera* camera, int width, int height) :
	Shader(),
	m_camera(camera),
	m_width(width),
	m_height(height),
	m_matrixBuffer(0)
{
}

SSAOShader::~SSAOShader()
{
}

bool SSAOShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* filename = "../Teapot/data/shaders/ssao.sb";
	vs_stream.open(filename, std::ifstream::in | std::ifstream::binary);
	if(vs_stream.good())
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
		output("\n!!!CAN'T FIND %s!!!\n\n", filename);
		return false;
	}

	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(D3DXMATRIX);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&bufferDesc, NULL, &m_matrixBuffer);

	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(D3DXVECTOR4) * KERNEL_SIZE;
	bufferDesc.CPUAccessFlags = 0;
	D3DXVECTOR4 kernelPoints[KERNEL_SIZE];
	for (int i = 0; i < KERNEL_SIZE; ++i)
	{
		kernelPoints[i] = D3DXVECTOR4(random(-1.0f, 1.0),
									  random(-1.0f, 1.0),
									  random(0.0f, 1.0), 0);
		kernelPoints[i] = normalize(kernelPoints[i]);
		kernelPoints[i] *= random(0.0f, 1.0);

		// pack the points towards the origin
		float scale = float(i) / float(KERNEL_SIZE);
		scale *= scale;
		scale = (1.0f - scale) * 0.1f + scale;
		kernelPoints[i] *= scale;
	}
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = kernelPoints;
	device->CreateBuffer(&bufferDesc, &data, &m_kernelBuffer);

	D3DXVECTOR4	randomRotations[ROTATION_COUNT];
	for (int i = 0; i < ROTATION_COUNT; ++i)
	{
		randomRotations[i] = D3DXVECTOR4(random(-1.0f, 1.0f),
										 random(-1.0f, 1.0f),
										 0, 0);
		randomRotations[i] = normalize(randomRotations[i]);
	}
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = ROTATION_TEX_DIM;
	textureDesc.Height = ROTATION_TEX_DIM;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	data.pSysMem = randomRotations;
	data.SysMemPitch = sizeof(D3DXVECTOR4) * ROTATION_TEX_DIM;
	device->CreateTexture2D(&textureDesc, &data, &m_randomRotationsTexture);
	device->CreateShaderResourceView(m_randomRotationsTexture, NULL, &m_randomRotations);

	return true;
}

void SSAOShader::render(int indexCount, int instanceCount)
{
	m_deviceContext->CSSetShader(m_computeShader, NULL, 0);

	D3DXMATRIX projection;
	m_camera->getProjectionMatrix(projection);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	D3DXMATRIX* matrixData = (D3DXMATRIX*)mappedResource.pData;
	D3DXMatrixTranspose(matrixData, &projection);
	m_deviceContext->Unmap(m_matrixBuffer, 0);
	ID3D11Buffer* constants[] = {m_matrixBuffer, m_kernelBuffer};
	m_deviceContext->CSSetConstantBuffers(0, 2, constants);
	m_deviceContext->CSSetShaderResources(2, 1, &m_randomRotations);
	m_deviceContext->Dispatch((m_width + THREADS_PER_GROUP_X + 1) / THREADS_PER_GROUP_X, (m_height + THREADS_PER_GROUP_Y + 1) / THREADS_PER_GROUP_Y, 1);
}