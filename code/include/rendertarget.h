#ifndef _RENDERTARGET_H_
#define _RENDERTARGET_H_

#include <d3d11.h>
#include <d3dx10math.h>
#include <stdint.h>
#include <vector>

#define RT_FLAG_CPU_READ (1 << 0)
#define RT_FLAG_UAV (1 << 1)

class RenderTarget
{
public:
	RenderTarget(ID3D11Device* device, ID3D11DeviceContext* deviceContext, 
		int width, int height, DXGI_FORMAT colorFormat, DXGI_FORMAT depthFormat, uint32_t flags, uint32_t arraySize = 1);
	~RenderTarget();

	void clear(D3DXVECTOR4 color, float depth);
	ID3D11ShaderResourceView* getShaderResourceView() { return m_shaderResourceView; };
	ID3D11ShaderResourceView* getDepthShaderResourceView() { return m_depthShaderResourceView; };
	ID3D11UnorderedAccessView* getUnorderedAccessView();
	void getData(void** ppData);
	
	void setViewport();

	ID3D11RenderTargetView* getRenderTargetView();
	ID3D11DepthStencilView* getDepthStencilView();

	void setActiveColorSlice(uint32_t activeColorSlice) { m_activeColorSlice = activeColorSlice; };
	void setActiveDepthSlice(uint32_t activeDepthSlice) { m_activeDepthSlice = activeDepthSlice; };

	uint32_t getWidth() { return m_width; };
	uint32_t getHeight() { return m_height; };

private:
	ID3D11DeviceContext* m_deviceContext;
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11Texture2D* m_stagingTexture;
	std::vector<ID3D11RenderTargetView*> m_renderTargetViews;
	std::vector<ID3D11DepthStencilView*> m_depthStencilViews;
	ID3D11ShaderResourceView* m_shaderResourceView;
	ID3D11ShaderResourceView* m_depthShaderResourceView;
	ID3D11Texture2D* m_depthStencilBuffer;

	ID3D11UnorderedAccessView* m_unorderedAccessView;

	D3D11_VIEWPORT m_viewport;
	ID3D11Device* m_device;

	uint32_t m_activeColorSlice;
	uint32_t m_activeDepthSlice;

	uint32_t m_width;
	uint32_t m_height;
};

#endif