#include "../../include/shaderrenderers/skyboxshader.h"
#include "../../include/camera.h"
#include "../../include/funcs.h"

SkyboxShader::SkyboxShader(Camera* camera) :
	Shader(),
	m_vertexShader(0),
	m_pixelShader(0),
	m_camera(camera),
	m_matrixBuffer(0),
	m_vertexBuffer(0)
{
}


SkyboxShader::~SkyboxShader()
{
}

bool SkyboxShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* file = "shaders/skyboxVS.sb";
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

	D3D11_INPUT_ELEMENT_DESC polygonLayout;
	polygonLayout.SemanticName = "POSITION";
	polygonLayout.SemanticIndex = 0;
	polygonLayout.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout.InputSlot = 0;
	polygonLayout.AlignedByteOffset = 0;
	polygonLayout.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout.InstanceDataStepRate = 0;

	device->CreateInputLayout(&polygonLayout, 1, vs_data, vs_size, &m_layout);

	file = "shaders/skyboxPS.sb";
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

	D3D11_BUFFER_DESC vertexBufferDesc;

    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(D3DXVECTOR3) * 36;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = cubeVertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

    device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

	return true;
}

void SkyboxShader::render(int indexCount, int instanceCount)
{
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX projection;

	D3DXVECTOR3 camPos;
	m_camera->getPosition(camPos);
	//D3DXMATRIX translation;
	D3DXMatrixTranslation(&world, camPos.x, camPos.y, camPos.z);

	m_camera->getViewMatrix(view);
	m_camera->getProjectionMatrix(projection);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	MatrixBuffer* matrixData = (MatrixBuffer*)mappedResource.pData;
	D3DXMatrixTranspose(&(matrixData->world), &world);
	D3DXMatrixTranspose(&(matrixData->view), &view);
	D3DXMatrixTranspose(&(matrixData->projection), &projection);
    m_deviceContext->Unmap(m_matrixBuffer, 0);

    m_deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);

	unsigned int stride[] = {sizeof(D3DXVECTOR3)}; 
	unsigned int offset[] = {0};
	ID3D11Buffer* vertexBuffers[] = {m_vertexBuffer};
	m_deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, stride, offset);

	m_deviceContext->IASetInputLayout(m_layout);
	m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    m_deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->Draw(36, 0);

	ID3D11ShaderResourceView* temp[1];
	temp[0] = 0;
	m_deviceContext->PSSetShaderResources(0, 1, temp);
}
