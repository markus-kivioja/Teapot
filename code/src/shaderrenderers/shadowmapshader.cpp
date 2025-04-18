#include "../../include/shaderrenderers/shadowmapshader.h"
#include "../../include/lightmanager.h"
#include "../../include/funcs.h"
#include <fstream>

ShadowMapShader::ShadowMapShader(float nearPlane, float farPlane) :
	Shader(),
	m_vertexShader(0),
	m_layout(0),
	m_matrixBuffer(0),
	m_spotLight(0),
	m_nearPlane(nearPlane),
	m_farPlane(farPlane)
{
}

ShadowMapShader::~ShadowMapShader()
{
	m_matrixBuffer->Release();
	m_layout->Release();
	m_vertexShader->Release();
	m_matrixBuffer = 0;
	m_layout = 0;
	m_vertexShader = 0;
}

bool ShadowMapShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* file = "shaders/shadowmapVS.sb";
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

	D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
	unsigned int numElements;

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

    numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	device->CreateInputLayout(polygonLayout, numElements, vs_data, vs_size, &m_layout);

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

void ShadowMapShader::render(int indexCount, int instanceCount)
{
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 position = D3DXVECTOR3(m_spotLight->positionSin.x, m_spotLight->positionSin.y, m_spotLight->positionSin.z);
	D3DXVECTOR3 direction = D3DXVECTOR3(m_spotLight->directionCos.x, m_spotLight->directionCos.y, m_spotLight->directionCos.z);
	D3DXVECTOR3 lookAt = position + direction;
	D3DXMATRIX view;
	D3DXMatrixLookAtLH(&view, &position, &lookAt, &up);

	D3DXMATRIX projection;
	D3DXMatrixPerspectiveFovLH(&projection, asin(m_spotLight->positionSin.w) * 2, 1.0f, m_nearPlane, m_farPlane);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBuffer* matrixData = (MatrixBuffer*)mappedResource.pData;
	D3DXMatrixTranspose(&(matrixData->world), &m_worldMatrix);
	D3DXMatrixTranspose(&(matrixData->view), &view);
	D3DXMatrixTranspose(&(matrixData->projection), &projection);
    m_deviceContext->Unmap(m_matrixBuffer, 0);

    m_deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	m_deviceContext->IASetInputLayout(m_layout);
    m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    m_deviceContext->PSSetShader(NULL, NULL, 0);
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}