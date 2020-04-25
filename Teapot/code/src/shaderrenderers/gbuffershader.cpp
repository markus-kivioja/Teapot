#include "../../include/shaderrenderers/gbuffershader.h"
#include "../../include/funcs.h"
#include "../../include/camera.h"
#include "../../include/constant_parameters.h"
#include <fstream>
#include <stdint.h>

GBufferShader::GBufferShader(Camera* camera, D3DXVECTOR4* normalBufferData, D3DXVECTOR4* uvBufferData) :
	Shader(),
	m_camera(camera),
	m_vertexShader(0),
	m_layout(0),
	m_sampleStateWrap(0),
	m_vsConstantBuffer(0),
	m_psConstantBuffer(0),
	m_uvBufferData(uvBufferData),
	m_normalBufferData(normalBufferData),
	m_puddleMask(0)
{
	for (int i = 0; i < TECHNIQUE_COUNT; ++i)
		m_pixelShaders[i] = 0;
}


GBufferShader::~GBufferShader()
{
	if (m_vsConstantBuffer)
		m_vsConstantBuffer->Release();
	m_vsConstantBuffer = 0;
	if (m_psConstantBuffer)
		m_psConstantBuffer->Release();
	m_psConstantBuffer = 0;
	if (m_sampleStateWrap)
		m_sampleStateWrap->Release();
	m_sampleStateWrap = 0;
	if (m_layout)
		m_layout->Release();
	m_layout = 0;
	for (int i = 0; i < TECHNIQUE_COUNT; ++i)
	{
		if (m_pixelShaders[i])
			m_pixelShaders[i]->Release();
		m_pixelShaders[i] = 0;
	}
	if (m_vertexShader)
		m_vertexShader->Release();
	m_vertexShader = 0;
}

bool GBufferShader::init(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_deviceContext = deviceContext;

	const unsigned int numElements = 4;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[numElements];
	
	std::ifstream vs_stream;
	size_t vs_size;
	char* vs_data;

	char* file = "../Teapot/data/shaders/gbufferVS.sb";
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

	HRESULT result;
	D3DX11CreateShaderResourceViewFromFile(device, L"../Teapot/data/textures/puddleMask.dds", NULL, NULL, &m_puddleMask, &result);
	if(FAILED(result))
		output("LOADING PUDDLE MASK FAILED\n");

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

	polygonLayout[3].SemanticName = "TANGENT";
	polygonLayout[3].SemanticIndex = 0;
	polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[3].InputSlot = 0;
	polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[3].InstanceDataStepRate = 0;

	device->CreateInputLayout(polygonLayout, numElements, vs_data, vs_size, &m_layout);

	file = "../Teapot/data/shaders/gbufferDribblePS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[DRIBBLE_TECHNIQUE]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}
	file = "../Teapot/data/shaders/gbufferRingPS.sb";
	vs_stream.open(file, std::ifstream::in | std::ifstream::binary);
	if (vs_stream.good())
	{
		vs_stream.seekg(0, std::ios::end);
		vs_size = size_t(vs_stream.tellg());
		vs_data = new char[vs_size];
		vs_stream.seekg(0, std::ios::beg);
		vs_stream.read(&vs_data[0], vs_size);
		vs_stream.close();
	
		device->CreatePixelShader(vs_data, vs_size, NULL, &m_pixelShaders[PUDDLE_TECHNIQUE]);	
	}
	else
	{
		output("\n!!!CAN'T FIND %s!!!\n\n", file);
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &m_sampleStateWrap);

	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(VSConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	device->CreateBuffer(&bufferDesc, NULL, &m_vsConstantBuffer);

	bufferDesc.ByteWidth = sizeof(PSConstantBuffer);
	device->CreateBuffer(&bufferDesc, NULL, &m_psConstantBuffer);

	bufferDesc.ByteWidth = sizeof(D3DXVECTOR4)*100;
	device->CreateBuffer(&bufferDesc, NULL, &m_waveBuffer);
	device->CreateBuffer(&bufferDesc, NULL, &m_dribbleBuffer);

	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(PuddleBuffer);
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	device->CreateBuffer(&bufferDesc, NULL, &m_puddleBuffer);
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_puddleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	PuddleBuffer* puddleData = (PuddleBuffer*)mappedResource.pData;
	uint32_t puddleCount = m_puddlePosSizes.size();
	if (puddleCount)
	{
		puddleData->puddlePosSizes[0] = D3DXVECTOR4((float)puddleCount, 0.0f, 0.0f, 0.0f);
		memcpy(&(puddleData->puddlePosSizes[1]), &(m_puddlePosSizes[0]), puddleCount*sizeof(D3DXVECTOR4));
	}
	m_deviceContext->Unmap(m_puddleBuffer, 0);

	return true;
}

void GBufferShader::render(int indexCount, int instanceCount)
{
	D3DXMATRIX view;
	D3DXMATRIX projection;

	m_camera->getViewMatrix(view);
	m_camera->getProjectionMatrix(projection);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_vsConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	VSConstantBuffer* vsData = (VSConstantBuffer*)mappedResource.pData;
	D3DXMatrixTranspose(&(vsData->world), &m_worldMatrix);
	D3DXMatrixTranspose(&(vsData->view), &view);
	D3DXMatrixTranspose(&(vsData->projection), &projection);
    m_deviceContext->Unmap(m_vsConstantBuffer, 0);
    m_deviceContext->VSSetConstantBuffers(0, 1, &m_vsConstantBuffer);

	m_deviceContext->Map(m_psConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	PSConstantBuffer* psData = (PSConstantBuffer*)mappedResource.pData;
	psData->shininess.x = m_shininess;
	D3DXMatrixTranspose(&(psData->view), &view);
    m_deviceContext->Unmap(m_psConstantBuffer, 0);

	ID3D11Buffer* psConstants[] = {m_psConstantBuffer, 
								   m_techniqueIndex == PUDDLE_TECHNIQUE ? m_waveBuffer : m_dribbleBuffer,
								   m_puddleBuffer};

    m_deviceContext->PSSetConstantBuffers(0, 3, psConstants);
	
	m_deviceContext->PSSetShaderResources(4, 1, &m_puddleMask);

	m_deviceContext->IASetInputLayout(m_layout);
    m_deviceContext->VSSetShader(m_vertexShader, NULL, 0);
    m_deviceContext->PSSetShader(m_pixelShaders[m_techniqueIndex], NULL, 0);
	m_deviceContext->PSSetSamplers(0, 1, &m_sampleStateWrap);
	m_deviceContext->CSSetSamplers(0, 1, &m_sampleStateWrap);
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}

bool pointInsideRect(D3DXVECTOR2 point, D3DXVECTOR4 rect)
{
	if (rect.x < point.x && point.x < rect.x + rect.z &&
		rect.y < point.y && point.y < rect.y + rect.w)
		return true;
	return false;
}

void GBufferShader::addWaveEffect(D3DXVECTOR3 pos)
{
	for (uint32_t i = 0; i < m_puddlePosSizes.size(); ++i)
		if (pointInsideRect(D3DXVECTOR2(pos.x, pos.z), m_puddlePosSizes[i]))
		{
			m_waveEffects.push_back(D3DXVECTOR4(pos, 0.0f));
			return;
		}
}

void GBufferShader::addDribbleEffect(D3DXVECTOR3 pos, D3DXVECTOR3 dir)
{
	m_dribbleEffects.push_back(D3DXVECTOR4(pos, 0.0f));
	m_dribbleDirs.push_back(dir);
}

void GBufferShader::updateEffects(int dt)
{
	uint32_t waveCount = m_waveEffects.size();
	for (uint32_t i = 0; i < waveCount; ++i)
	{
		m_waveEffects[i].w += dt;
		if (m_waveEffects[i].w > WAVE_LIFESPAN)
		{
			m_waveEffects.erase(m_waveEffects.begin() + i);
			i--;
			waveCount--;
		}
	}
	uint32_t dribbleCount = m_dribbleEffects.size();
	for (uint32_t i = 0; i < dribbleCount; ++i)
	{
		D3DXVECTOR4* ef = &(m_dribbleEffects[i]);
		*ef += D3DXVECTOR4(m_dribbleDirs[i] * DRIBBLE_SLIDE_SPEED * (float)dt / 16.0f, (float)dt);
		//int v = (int)((ef->pos.x + 5.0f) / 10.0f * BUFS_WIDTH);
		//int u = (int)((ef->pos.z + 5.0f) / 10.0f * BUFS_HEIGHT);
		//D3DXVECTOR4 uvBufData = m_uvBufferData[BUFS_WIDTH*v+u];
		//ef->uv = D3DXVECTOR2(uvBufData.x, uvBufData.y);

		//D3DXVECTOR4 gData = m_normalBufferData[2048*v+u];
		//D3DXVECTOR3 normal(gData.y, gData.z, gData.w);
		//float dx = normal.x*GBUF_LOOKUP_DISTANCE_FACTOR;
		//float dz = normal.z*GBUF_LOOKUP_DISTANCE_FACTOR;
		//v = (int)((ef->pos.x + dx + 5.0f) / 10.0f * BUFS_WIDTH);
		//u = (int)((ef->pos.z + dz + 5.0f) / 10.0f * BUFS_HEIGHT);
		//float dy = 5.0f-m_normalBufferData[BUFS_WIDTH*v+u].x + 1.0f - ef->pos.y;
		//ef->dir = D3DXVECTOR3(dx, dy, dz);
		if (ef->w > DRIBBLE_LIFESPAN)
		{
			m_dribbleEffects.erase(m_dribbleEffects.begin() + i);
			m_dribbleDirs.erase(m_dribbleDirs.begin() + i);
			i--;
			dribbleCount--;
		}
	}

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_waveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	D3DXVECTOR4* waves = (D3DXVECTOR4*)mappedResource.pData;
	if (waveCount)
	{
		waves[0] = D3DXVECTOR4((float)waveCount, 0.0f, 0.0f, 0.0f);
		memcpy(waves + 1, &(m_waveEffects[0]), waveCount*sizeof(D3DXVECTOR4));
	}
	m_deviceContext->Unmap(m_waveBuffer, 0);

	m_deviceContext->Map(m_dribbleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	D3DXVECTOR4* dribbles = (D3DXVECTOR4*)mappedResource.pData;
	if (dribbleCount)
	{
		dribbles[0] = D3DXVECTOR4((float)dribbleCount, 0.0f, 0.0f, 0.0f);
		memcpy(dribbles + 1, &(m_dribbleEffects[0]), dribbleCount*sizeof(D3DXVECTOR4));
	}
	m_deviceContext->Unmap(m_dribbleBuffer, 0);
}