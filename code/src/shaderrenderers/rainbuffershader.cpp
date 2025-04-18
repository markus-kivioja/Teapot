#include "../../include/shaderrenderers/rainbuffershader.h"
#include "../../include/funcs.h"
#include "../../include/camera.h"
#include "../../include/light.h"
#include <fstream>

RainBufferShader::RainBufferShader(Camera* camera) :
	Shader(),
	m_camera(camera),
	m_vertexShader(0),
	m_pixelShader(0),
	m_layout(0),
	m_matrixBuffer(0)
{
}

RainBufferShader::~RainBufferShader()
{
	m_matrixBuffer->Release();
	m_layout->Release();
	m_pixelShader->Release();
	m_vertexShader->Release();
	m_matrixBuffer = 0;
	m_layout = 0;
	m_pixelShader = 0;
	m_vertexShader = 0;
}

bool RainBufferShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* file = "shaders/rainbufVS.sb";
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

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	device->CreateInputLayout(polygonLayout, numElements, vs_data, vs_size, &m_layout);

	file = "shaders/rainbufPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShader);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
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

	return true;
}

void RainBufferShader::render(int indexCount, int instanceCount)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* matrixData;

	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 position(0.000001f, 6.0f, 0.0f);
	D3DXVECTOR3 lookAt(0.0f, 0.0f, 0.0f);

	D3DXMATRIX viewMatrix;
	D3DXMatrixLookAtLH(&viewMatrix, &position, &lookAt, &up);
	D3DXMATRIX projectionMatrix;
	D3DXMatrixOrthoLH(&projectionMatrix, 10.0f, 10.0f, 1.0f, 5.0001f);
	D3DXMATRIX worldMatrix;

	D3DXMatrixTranspose(&worldMatrix, &(m_worldMatrix));
	D3DXMatrixTranspose(&viewMatrix, &viewMatrix);
	D3DXMatrixTranspose(&projectionMatrix, &projectionMatrix);

	m_deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	matrixData = (MatrixBuffer*)mappedResource.pData;
	matrixData->world = worldMatrix;
	matrixData->view = viewMatrix;
	matrixData->projection = projectionMatrix;

    m_deviceContext->Unmap(m_matrixBuffer, 0);

    m_deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	m_deviceContext->IASetInputLayout(m_layout);
    m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    m_deviceContext->PSSetShader(m_pixelShader, NULL, 0);
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}