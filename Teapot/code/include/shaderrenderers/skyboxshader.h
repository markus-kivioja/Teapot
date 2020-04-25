#pragma once

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

class Camera;

const float cubeVertices[] =
{
	-1, -1, -1,
	-1, -1, 1,
	1, -1, -1,
	1, -1, -1,
	-1, -1, 1,
	1, -1, 1,

	-1, 1, -1,
	1, 1, -1,
	-1, 1, 1,
	-1, 1, 1,
	1, 1, -1,
	1, 1, 1,

	1, -1, -1,
	1, -1, 1,
	1, 1, -1,
	1, 1, -1,
	1, -1, 1,
	1, 1, 1,

	-1, -1, -1,
	-1, 1, -1,
	-1, -1, 1,
	-1, -1, 1,
	-1, 1, -1,
	-1, 1, 1,

	-1, -1, 1,
	-1, 1, 1,
	1, -1, 1,
	1, -1, 1,
	-1, 1, 1,
	1, 1, 1,

	-1, -1, -1,
	1, -1, -1,
	-1, 1, -1,
	-1, 1, -1,
	1, -1, -1,
	1, 1, -1
};

class SkyboxShader : public Shader
{
public:
	SkyboxShader(Camera* camera);
	~SkyboxShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

private:
	struct MatrixBuffer
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	ID3D11VertexShader*			m_vertexShader;
	ID3D11PixelShader*			m_pixelShader;
	ID3D11InputLayout*			m_layout;
	ID3D11Buffer*				m_matrixBuffer;
	ID3D11Buffer*				m_vertexBuffer;

	Camera*						m_camera;
};

