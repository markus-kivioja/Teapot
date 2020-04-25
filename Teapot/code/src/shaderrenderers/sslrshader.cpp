#include "../../include/shaderrenderers/sslrshader.h"
#include "../../include/camera.h"
#include "../../include/funcs.h"
#include <fstream>
#include <d3dx11async.h>

#define THREADS_PER_GROUP_X 16
#define THREADS_PER_GROUP_Y 16

SSLRShader::SSLRShader(Camera* camera, int width, int height) :
	Shader(),
	m_camera(camera),
	m_width(width),
	m_height(height),
	m_matrixBuffer(0)
{
}

SSLRShader::~SSLRShader()
{
}

bool SSLRShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* filename = "../Teapot/data/shaders/sslr.sb";
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

	D3D11_BUFFER_DESC matrixBufferDesc;
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(D3DXMATRIX);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	return true;
}

void SSLRShader::render(int indexCount, int instanceCount)
{
	m_deviceContext->CSSetShader(m_computeShader, NULL, 0);

	D3DXMATRIX projection;
	m_camera->getProjectionMatrix(projection);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	D3DXMATRIX* matrixData = (D3DXMATRIX*)mappedResource.pData;
	D3DXMatrixTranspose(matrixData, &projection);
	m_deviceContext->Unmap(m_matrixBuffer, 0);
	ID3D11Buffer* constants[] = {m_matrixBuffer};
	m_deviceContext->CSSetConstantBuffers(0, 1, constants);

	m_deviceContext->Dispatch((m_width + THREADS_PER_GROUP_X + 1) / THREADS_PER_GROUP_X, (m_height + THREADS_PER_GROUP_Y + 1) / THREADS_PER_GROUP_Y, 1);
}