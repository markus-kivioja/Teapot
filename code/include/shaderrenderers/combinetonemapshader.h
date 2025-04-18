#pragma once

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

class CombineTonemapShader : public Shader
{
public:
	CombineTonemapShader();
	~CombineTonemapShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);


private:
	ID3D11VertexShader*      m_vertexShader;
	ID3D11PixelShader*       m_pixelShader;
};

