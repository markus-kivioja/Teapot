#include "../../include/shaderrenderers/blitshader.h"
#include "../../include/funcs.h"

BlitShader::BlitShader() :
	Shader(),
	m_constantBuffer(0),
	m_arraySlice(0)
{
}


BlitShader::~BlitShader()
{
}

bool BlitShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* file = "shaders/blitVS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreateVertexShader(vs_data, vs_size, NULL, &m_vertexShaders[NORMAL_TECHNIQUE]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}

	file = "shaders/blitarrayVS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreateVertexShader(vs_data, vs_size, NULL, &m_vertexShaders[ARRAY_TECHNIQUE]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}

	file = "shaders/blitPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[NORMAL_TECHNIQUE]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}

	file = "shaders/blitarrayPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[ARRAY_TECHNIQUE]);	
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

void BlitShader::render(int indexCount, int instanceCount)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	m_deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	D3DXVECTOR4* data = (D3DXVECTOR4*)mappedResource.pData;
	*data = m_positionSize;
    m_deviceContext->Unmap(m_constantBuffer, 0);

	m_deviceContext->Map(m_sliceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	data = (D3DXVECTOR4*)mappedResource.pData;
	*data = D3DXVECTOR4((float)m_arraySlice, 0, 0, 0);
	m_deviceContext->Unmap(m_sliceBuffer, 0);

    m_deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	if (m_techniqueIndex == ARRAY_TECHNIQUE)
		m_deviceContext->VSSetConstantBuffers(1, 1, &m_sliceBuffer);

	m_deviceContext->VSSetShader(m_vertexShaders[m_techniqueIndex], NULL, 0);
    m_deviceContext->PSSetShader(m_pixelShaders[m_techniqueIndex], NULL, 0);

	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_deviceContext->Draw(indexCount, 0);

	ID3D11ShaderResourceView* temp[1];
	temp[0] = 0;
	m_deviceContext->PSSetShaderResources(0, 1, temp);
}
