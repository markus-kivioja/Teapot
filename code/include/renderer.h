#ifndef _RENDERER_H_
#define _RENDERER_H_

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dx10.lib")

#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3dx10math.h>

#define MAX_MRT_COUNT 8

class RenderTarget;

class Renderer
{
public:
	Renderer(int screenWidth, int screenHeight, HWND hwnd);
	~Renderer();
	
	void clear();
	void present();

	void setBackBufferRenderTarget();
	void resetViewport();

	void enableBlending();
	void disableBlending();

	void enableDepth();
	void disableDepth();

	void enableCull();
	void disableCull();

	void setDepthCompareLess();

	void setRenderTargets(RenderTarget* renderTargets[], int count);
	void setRenderAndDepthTargets(RenderTarget* renderTargets[], RenderTarget* depthTarget, int count);
	void setDepthTarget(RenderTarget* depthTarget);

	ID3D11Device* getDevice();
	ID3D11DeviceContext* getDeviceContext();

private:
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthEnabledState;
	ID3D11DepthStencilState* m_depthDisabledState;
	ID3D11DepthStencilState* m_depthCompareLessState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;
	ID3D11RasterizerState* m_rasterNoCull;
	D3D11_VIEWPORT m_viewport;
	ID3D11BlendState* m_blendEnabled;
	ID3D11BlendState* m_blendDisabled;
};

#endif