#pragma once

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

enum BlitTechnique
{
	NORMAL_TECHNIQUE,
	ARRAY_TECHNIQUE,
	BLIT_TECHNIQUE_COUNT
};

class BlitShader : public Shader
{
public:
	BlitShader();
	~BlitShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

	void setPositionSize(const D3DXVECTOR4& posSize) { m_positionSize = posSize; };

	void setArraySlice(int arraySlice) { m_arraySlice = arraySlice; };

private:
	ID3D11Buffer*            m_constantBuffer;
	ID3D11Buffer*			 m_sliceBuffer;
	ID3D11VertexShader*      m_vertexShaders[BLIT_TECHNIQUE_COUNT];
	ID3D11PixelShader*       m_pixelShaders[BLIT_TECHNIQUE_COUNT];

	int						 m_arraySlice;

	D3DXVECTOR4				 m_positionSize;
};

