#include "../include/renderer.h"
#include "../include/constant_parameters.h"
#include "../include/rendertarget.h"
#include "../include/funcs.h"

Renderer::Renderer(int screenWidth, int screenHeight, HWND hwnd)
{
	IDXGIFactory* factory;
	CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);

	IDXGIAdapter* adapter;
	factory->EnumAdapters(0, &adapter);

	IDXGIOutput* adapterOutput;
	adapter->EnumOutputs(0, &adapterOutput);

	unsigned int numModes, i, numerator, denominator;
	adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);

	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];

	adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);

	for(i = 0; i < numModes; ++i)
	{
		if(displayModeList[i].Width == (unsigned int)screenWidth)
		{
			if(displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}
	DXGI_ADAPTER_DESC adapterDesc;
	adapter->GetDesc(&adapterDesc);

	delete [] displayModeList;
	displayModeList = 0;
	adapterOutput->Release();
	adapterOutput = 0;
	adapter->Release();
	adapter = 0;
	factory->Release();
	factory = 0;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
    memset(&swapChainDesc, 0, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	if (VSYNC)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = !FULLSCREEN;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, 
										   D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	
	ID3D11Texture2D* backBuffer;	
	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView);
	backBuffer->Release();
	backBuffer = 0;

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	memset(&depthBufferDesc, 0, sizeof(depthBufferDesc));
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	memset(&depthStencilDesc, 0, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
#ifdef REVERSE_DEPTH
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
#else
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
#endif
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthEnabledState);

	m_deviceContext->OMSetDepthStencilState(m_depthEnabledState, 1);

	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthCompareLessState);

	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.StencilEnable = false;
	m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthDisabledState);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	memset(&depthStencilViewDesc, 0, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	m_deviceContext->RSSetState(m_rasterState);

	rasterDesc.CullMode = D3D11_CULL_NONE;
	m_device->CreateRasterizerState(&rasterDesc, &m_rasterNoCull);

	D3D11_BLEND_DESC blendStateDesc;
	memset(&blendStateDesc, 0, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	m_device->CreateBlendState(&blendStateDesc, &m_blendEnabled);

	blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
	
	m_device->CreateBlendState(&blendStateDesc, &m_blendDisabled);

    m_viewport.Width = (float)screenWidth;
    m_viewport.Height = (float)screenHeight;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;

    m_deviceContext->RSSetViewports(1, &m_viewport);
}

Renderer::~Renderer()
{
	m_swapChain->SetFullscreenState(false, NULL);
	m_rasterState->Release();
	m_depthStencilView->Release();
	m_depthEnabledState->Release();
	m_depthDisabledState->Release();
	m_depthStencilBuffer->Release();
	m_renderTargetView->Release();
	m_blendDisabled->Release();
	m_blendEnabled->Release();
	m_deviceContext->Release();
	m_device->Release();
	m_swapChain->Release();
	m_renderTargetView = 0;
	m_deviceContext = 0;
	m_device = 0;
	m_swapChain = 0;
	m_depthStencilBuffer = 0;
	m_depthEnabledState = 0;
	m_depthDisabledState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
}

void Renderer::enableBlending()
{
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_deviceContext->OMSetBlendState(m_blendEnabled, blendFactor, 0xFFFFFFFF);
}

void Renderer::disableBlending()
{
	float blendFactor[] = {0.0f, 0.0f, 0.0f, 0.0f};
	m_deviceContext->OMSetBlendState(m_blendDisabled, blendFactor, 0xFFFFFFFF);
}

void Renderer::enableDepth()
{
	m_deviceContext->OMSetDepthStencilState(m_depthEnabledState, 1);
}

void Renderer::disableDepth()
{
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledState, 1);
}

void Renderer::enableCull()
{
	m_deviceContext->RSSetState(m_rasterState);
}

void Renderer::disableCull()
{
	m_deviceContext->RSSetState(m_rasterNoCull);
}

void Renderer::setDepthCompareLess()
{
	m_deviceContext->OMSetDepthStencilState(m_depthCompareLessState, 1);
}

void Renderer::clear()
{
#ifdef REVERSE_DEPTH
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f));
#else
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
#endif
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 0.0f, 0);
}

void Renderer::present()
{
	if (VSYNC)
		m_swapChain->Present(1, 0);
	else
		m_swapChain->Present(0, 0);
}

ID3D11Device* Renderer::getDevice()
{
	return m_device;
}

ID3D11DeviceContext* Renderer::getDeviceContext()
{
	return m_deviceContext;
}

void Renderer::setBackBufferRenderTarget()
{
	ID3D11RenderTargetView* renderTargets[MAX_MRT_COUNT];
	renderTargets[0] = m_renderTargetView;
	for (int i = 1; i < MAX_MRT_COUNT; ++i)
		renderTargets[i] = 0;
	m_deviceContext->OMSetRenderTargets(MAX_MRT_COUNT, renderTargets, m_depthStencilView);
}

void Renderer::setDepthTarget(RenderTarget* depthTarget)
{
	if (depthTarget)
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, depthTarget->getDepthStencilView());
	else
		m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, NULL);
}

void Renderer::resetViewport()
{
    m_deviceContext->RSSetViewports(1, &m_viewport);
}

void Renderer::setRenderTargets(RenderTarget* renderTargets[], int count)
{	
	ID3D11RenderTargetView* renderTargetViews[MAX_MRT_COUNT];
	for (int i = 0; i < count; ++i)
	{
		if (renderTargets[i])
			renderTargetViews[i] = renderTargets[i]->getRenderTargetView();
		else
			renderTargetViews[i] = NULL;
	}
	ID3D11DepthStencilView* depthView = NULL;
	if (renderTargets[0])
	{
		renderTargets[0]->setViewport();
		depthView = renderTargets[0]->getDepthStencilView();
	}
	m_deviceContext->OMSetRenderTargets(count, renderTargetViews, depthView);
}

void Renderer::setRenderAndDepthTargets(RenderTarget* renderTargets[], RenderTarget* depthTarget, int count)
{
		ID3D11RenderTargetView* renderTargetViews[MAX_MRT_COUNT];
	for (int i = 0; i < count; ++i)
	{
		if (renderTargets[i])
			renderTargetViews[i] = renderTargets[i]->getRenderTargetView();
		else
			renderTargetViews[i] = NULL;
	}
	ID3D11DepthStencilView* depthView = NULL;
	if (renderTargets[0])
		renderTargets[0]->setViewport();
	if (depthTarget)
		depthView = depthTarget->getDepthStencilView();
	m_deviceContext->OMSetRenderTargets(count, renderTargetViews, depthView);
}