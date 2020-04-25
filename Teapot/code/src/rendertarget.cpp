#include "../include/rendertarget.h"
#include <stdio.h>

RenderTarget::RenderTarget(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
						   int textureWidth, int textureHeight, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat, uint32_t flags, uint32_t arraySize) :
	m_device(device),
	m_deviceContext(deviceContext),
	m_renderTargetTexture(0),
	m_shaderResourceView(0),
	m_depthStencilBuffer(0),
	m_stagingTexture(0),
	m_unorderedAccessView(0),
	m_width(textureWidth),
	m_height(textureHeight),
	m_activeColorSlice(0),
	m_activeDepthSlice(0)
{
	if (colorFormat != DXGI_FORMAT_UNKNOWN)
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		ZeroMemory(&textureDesc, sizeof(textureDesc));

		textureDesc.Width = textureWidth;
		textureDesc.Height = textureHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = arraySize;
		textureDesc.Format = colorFormat;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		if (flags | RT_FLAG_UAV)
			textureDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		device->CreateTexture2D(&textureDesc, NULL, &m_renderTargetTexture);

		if (flags | RT_FLAG_CPU_READ)
		{
			textureDesc.Usage = D3D11_USAGE_STAGING;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			textureDesc.BindFlags = 0;
			m_device->CreateTexture2D(&textureDesc, NULL, &m_stagingTexture);
		}

		renderTargetViewDesc.Format = textureDesc.Format;
		if (arraySize > 1)
		{
			renderTargetViewDesc.Texture2DArray.MipSlice = 0;
			renderTargetViewDesc.Texture2DArray.ArraySize = 1;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			for (uint32_t i = 0; i < arraySize; ++i)
			{
				renderTargetViewDesc.Texture2DArray.FirstArraySlice = i;
				m_renderTargetViews.push_back(NULL);
				device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetViews[i]);
			}
		}
		else
		{
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			m_renderTargetViews.push_back(NULL);
			device->CreateRenderTargetView(m_renderTargetTexture, &renderTargetViewDesc, &m_renderTargetViews[0]);
		}

		shaderResourceViewDesc.Format = textureDesc.Format;
		if (arraySize > 1)
		{
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
			shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
			shaderResourceViewDesc.Texture2DArray.ArraySize = arraySize;
		}
		else
		{
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;
		}
		device->CreateShaderResourceView(m_renderTargetTexture, &shaderResourceViewDesc, &m_shaderResourceView);

		if (flags | RT_FLAG_UAV)
			m_device->CreateUnorderedAccessView(m_renderTargetTexture, NULL, &m_unorderedAccessView);
	}

	if (depthFormat != DXGI_FORMAT_UNKNOWN)
	{
		D3D11_TEXTURE2D_DESC depthBufferDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

		depthBufferDesc.Width = textureWidth;
		depthBufferDesc.Height = textureHeight;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = arraySize;
		depthBufferDesc.Format = DXGI_FORMAT_R32G8X24_TYPELESS;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthBufferDesc.CPUAccessFlags = 0;
		depthBufferDesc.MiscFlags = 0;
		device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);

		ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

		depthStencilViewDesc.Format = depthFormat;
		if (arraySize > 1)
		{
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			depthStencilViewDesc.Texture2DArray.MipSlice = 0;
			depthStencilViewDesc.Texture2DArray.ArraySize = 1;
			for (uint32_t i = 0; i < arraySize; ++i)
			{
				depthStencilViewDesc.Texture2DArray.FirstArraySlice = i;
				m_depthStencilViews.push_back(NULL);
				device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilViews[i]);
			}
		}
		else
		{
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;
			m_depthStencilViews.push_back(NULL);
			device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilViews[0]);
		}


		shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
		if (arraySize > 1)
		{
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			shaderResourceViewDesc.Texture2DArray.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2DArray.MipLevels = 1;
			shaderResourceViewDesc.Texture2DArray.FirstArraySlice = 0;
			shaderResourceViewDesc.Texture2DArray.ArraySize = arraySize;
		}
		else
		{
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;
		}
		device->CreateShaderResourceView(m_depthStencilBuffer, &shaderResourceViewDesc, &m_depthShaderResourceView);
	}

    m_viewport.Width = (float)textureWidth;
    m_viewport.Height = (float)textureHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
}

RenderTarget::~RenderTarget()
{
	uint32_t depthStencilCount = m_depthStencilViews.size();
	for (uint32_t i = 0; i < depthStencilCount; ++i)
		if (m_depthStencilViews[i])
			m_depthStencilViews[i]->Release();
	if (m_depthStencilBuffer)
		m_depthStencilBuffer->Release();
	if (m_shaderResourceView)
		m_shaderResourceView->Release();
	uint32_t renderTargetCount = m_renderTargetViews.size();
	for (uint32_t i = 0; i < renderTargetCount; ++i)
		if (m_renderTargetViews[i])
			m_renderTargetViews[i]->Release();
	if (m_renderTargetTexture)
		m_renderTargetTexture->Release();
	if (m_depthStencilBuffer)
		m_depthStencilBuffer = 0;
	if (m_shaderResourceView)
		m_shaderResourceView = 0;
	if (m_renderTargetTexture)
		m_renderTargetTexture = 0;
}

void RenderTarget::getData(void** data)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	m_stagingTexture->GetDesc(&textureDesc);
	
	m_deviceContext->CopyResource(m_stagingTexture, m_renderTargetTexture);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	m_deviceContext->Map(m_stagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
	
	*data = new float[4*textureDesc.Width*textureDesc.Height];
	memcpy(*data, mappedResource.pData, mappedResource.RowPitch*textureDesc.Height);

	m_deviceContext->Unmap(m_stagingTexture, 0);
}

void RenderTarget::clear(D3DXVECTOR4 color, float depth)
{
	uint32_t renderTargetCount = m_renderTargetViews.size();
	for (uint32_t i = 0; i < renderTargetCount; ++i)
		m_deviceContext->ClearRenderTargetView(m_renderTargetViews[i], color);
	uint32_t depthStencilCount = m_depthStencilViews.size();
	for (uint32_t i = 0; i < depthStencilCount; ++i)
		m_deviceContext->ClearDepthStencilView(m_depthStencilViews[i], D3D11_CLEAR_DEPTH, depth, 0);
}

ID3D11UnorderedAccessView* RenderTarget::getUnorderedAccessView()
{
	return m_unorderedAccessView;
}

void RenderTarget::setViewport()
{
	m_deviceContext->RSSetViewports(1, &m_viewport);
}

ID3D11RenderTargetView* RenderTarget::getRenderTargetView()
{
	if (m_activeColorSlice < m_renderTargetViews.size())
		return m_renderTargetViews[m_activeColorSlice];
	else
		return NULL;
}

ID3D11DepthStencilView* RenderTarget::getDepthStencilView()
{
	if (m_activeDepthSlice < m_depthStencilViews.size())
		return m_depthStencilViews[m_activeDepthSlice];
	else
		return NULL;
}