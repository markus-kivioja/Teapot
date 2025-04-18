#include "../../include/shaderrenderers/blurshader.h"
#include "../../include/funcs.h"

BlurShader::BlurShader() :
	Shader(),
	m_constantBuffer(0),
	m_sigma(0),
	m_luminanceThreshold(0)
{
}


BlurShader::~BlurShader()
{
}

bool BlurShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* file = "shaders/blurVS.sb";
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
	file = "shaders/blurXPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[X_TECHNIQUE]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}
	file = "shaders/blurYPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[Y_TECHNIQUE]);	
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

void BlurShader::render(int indexCount, int instanceCount)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	m_deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	D3DXVECTOR4* data = (D3DXVECTOR4*)mappedResource.pData;
	*data = m_pixelSize;
	data->z = m_sigma;
	data->w = m_luminanceThreshold;
	m_deviceContext->Unmap(m_constantBuffer, 0);
	m_deviceContext->PSSetConstantBuffers(0, 1, &m_constantBuffer);

	m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	m_deviceContext->PSSetShader(m_pixelShaders[m_techniqueIndex], NULL, 0);

	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_deviceContext->Draw(4, 0);
}
