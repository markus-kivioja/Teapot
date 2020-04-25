#pragma once

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

enum DownsampleTechnique
{
	TECHNIQUE_2x2,
	TECHNIQUE_4x4,
	DOWNSAMPLE_TECHNIQUE_COUNT
};

class DownsampleShader : public Shader
{
public:
	DownsampleShader();
	~DownsampleShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

	void setPixelSize(D3DXVECTOR4 size) { m_pixelSize = size; };

private:
	ID3D11Buffer*            m_constantBuffer;
	ID3D11Buffer*			 m_sliceBuffer;
	ID3D11VertexShader*      m_vertexShader;
	ID3D11PixelShader*       m_pixelShaders[DOWNSAMPLE_TECHNIQUE_COUNT];

	D3DXVECTOR4				 m_pixelSize;
};

