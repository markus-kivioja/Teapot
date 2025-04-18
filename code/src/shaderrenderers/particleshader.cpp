#include "../../include/shaderrenderers/particleshader.h"
#include "../../include/constant_parameters.h"
#include "../../include/window.h"
#include "../../include/camera.h"
#include "../../include/light.h"
#include "../../include/funcs.h"
#include <fstream>

ParticleShader::ParticleShader(Camera* camera) :
	Shader(),
	m_camera(camera),
	m_vertexShader(0),
	m_layout(0),
	m_matrixBuffer(0),
	m_sampleStateClamp(0),
	m_depthState(0)
{
	for (int i = 0; i < TYPE_COUNT; ++i)
		m_pixelShaders[i] = 0;
}

ParticleShader::~ParticleShader()
{
	if (m_matrixBuffer)
		m_matrixBuffer->Release();
	if (m_sampleStateClamp)
		m_sampleStateClamp->Release();
	if (m_layout)
		m_layout->Release();
	if (m_vertexShader)
		m_vertexShader->Release();
	if (m_depthState)
		m_depthState->Release();
	m_matrixBuffer = 0;
	m_sampleStateClamp = 0;
	m_layout = 0;
	m_vertexShader = 0;
	m_depthState = 0;

	for (int i = 0; i < TYPE_COUNT; ++i)
		if (m_pixelShaders[i])
		{
			m_pixelShaders[i]->Release();
			m_pixelShaders[i] = 0;
		}
}

bool ParticleShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* file = "shaders/particleVS.sb";
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

    D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	
	const unsigned int numElements = 4;
	D3D11_INPUT_ELEMENT_DESC vbLayout[numElements];
	vbLayout[0].SemanticName = "POSITION";
	vbLayout[0].SemanticIndex = 0;
	vbLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vbLayout[0].InputSlot = 0;
	vbLayout[0].AlignedByteOffset = 0;
	vbLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vbLayout[0].InstanceDataStepRate = 0;

	vbLayout[1].SemanticName = "TEXCOORD";
	vbLayout[1].SemanticIndex = 0;
	vbLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	vbLayout[1].InputSlot = 0;
	vbLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vbLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	vbLayout[1].InstanceDataStepRate = 0;

	vbLayout[2].SemanticName = "TEXCOORD";
	vbLayout[2].SemanticIndex = 1;
	vbLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vbLayout[2].InputSlot = 1;
	vbLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vbLayout[2].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	vbLayout[2].InstanceDataStepRate = 1;

	vbLayout[3].SemanticName = "TEXCOORD";
	vbLayout[3].SemanticIndex = 2;
	vbLayout[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vbLayout[3].InputSlot = 1;
	vbLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	vbLayout[3].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	vbLayout[3].InstanceDataStepRate = 1;

	device->CreateInputLayout(vbLayout, numElements, vs_data, vs_size, &m_layout);

	file = "shaders/particleDropPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[DROP]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}

	file = "shaders/particleSplashPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[SPLASH]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}

	file = "shaders/particlePointLightPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[POINT_LIGHT]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}
	
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &m_sampleStateClamp);

    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
#ifdef REVERSE_DEPTH
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER;
#else
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
#endif
	depthStencilDesc.StencilEnable = false;

	device->CreateDepthStencilState(&depthStencilDesc, &m_depthState);

	return true;
}

void ParticleShader::render(int indexCount, int instanceCount)
{	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* matrixData;

	D3DXMATRIX view;
	D3DXMATRIX projection;
	D3DXMATRIX lightView;
	D3DXMATRIX lightProjection;

	m_camera->getViewMatrix(view);
	m_camera->getProjectionMatrix(projection);

	m_deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	matrixData = (MatrixBuffer*)mappedResource.pData;
	D3DXMatrixTranspose(&(matrixData->world), &m_worldMatrix);
	D3DXMatrixTranspose(&(matrixData->view), &view);
	D3DXMatrixTranspose(&(matrixData->projection), &projection);
    m_deviceContext->Unmap(m_matrixBuffer, 0);

    m_deviceContext->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	
	m_deviceContext->IASetInputLayout(m_layout);
    m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    m_deviceContext->PSSetShader(m_pixelShaders[m_techniqueIndex], NULL, 0);
	m_deviceContext->PSSetSamplers(0, 1, &m_sampleStateClamp);

	m_deviceContext->OMSetDepthStencilState(m_depthState, 1);

	m_deviceContext->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}