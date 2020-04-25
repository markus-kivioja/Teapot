#include "../../include/shaderrenderers/downsampleshader.h"
#include "../../include/funcs.h"

DownsampleShader::DownsampleShader() :
	Shader(),
	m_constantBuffer(0)
{
}


DownsampleShader::~DownsampleShader()
{
}

bool DownsampleShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* file = "../Teapot/data/shaders/downsampleVS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreateVertexShader(vs_data, vs_size, NULL, &m_vertexShader);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}
	file = "../Teapot/data/shaders/downsample2x2PS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[TECHNIQUE_2x2]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}
	file = "../Teapot/data/shaders/downsample4x4PS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[TECHNIQUE_4x4]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}

	D3D11_BUFFER_DESC constBufferDesc;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.ByteWidth = sizeof(D3DXVECTOR4);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.MiscFlags = 0;
	constBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&constBufferDesc, NULL, &m_constantBuffer);
	device->CreateBuffer(&constBufferDesc, NULL, &m_sliceBuffer);

	return true;
}

void DownsampleShader::render(int indexCount, int instanceCount)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	D3DXVECTOR4* data = (D3DXVECTOR4*)mappedResource.pData;
	*data = m_pixelSize;
	m_deviceContext->Unmap(m_constantBuffer, 0);
	m_deviceContext->PSSetConstantBuffers(0, 1, &m_constantBuffer);

	m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	m_deviceContext->PSSetShader(m_pixelShaders[m_techniqueIndex], NULL, 0);

	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_deviceContext->Draw(4, 0);

	ID3D11ShaderResourceView* temp[1];
	temp[0] = 0;
	m_deviceContext->PSSetShaderResources(0, 1, temp);
}
