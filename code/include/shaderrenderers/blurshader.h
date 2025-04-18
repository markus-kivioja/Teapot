#pragma once

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

enum BloomTechnique
{
	X_TECHNIQUE,
	Y_TECHNIQUE,
	BLOOM_TECHNIQUE_COUNT
};


class BlurShader : public Shader
{
public:
	BlurShader();
	~BlurShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

	void setPixelSize(D3DXVECTOR4 pixelSize) { m_pixelSize = pixelSize; };

	void setLuminanceThreshold(float luminanceThreshold) { m_luminanceThreshold = luminanceThreshold; };
	void setSigma(float sigma) { m_sigma = sigma; };

private:
	float					 m_luminanceThreshold;
	float					 m_sigma;
	D3DXVECTOR2				 m_direction;

	ID3D11Buffer*            m_constantBuffer;
	ID3D11Buffer*			 m_sliceBuffer;
	ID3D11VertexShader*      m_vertexShader;
	ID3D11PixelShader*       m_pixelShaders[BLOOM_TECHNIQUE_COUNT];

	D3DXVECTOR4				 m_pixelSize;
};

