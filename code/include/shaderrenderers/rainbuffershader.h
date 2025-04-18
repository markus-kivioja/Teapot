#ifndef _RAINBUFFERSHADER_H_
#define _RAINBUFFERSHADER_H_

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

class Camera;

class RainBufferShader : public Shader
{
public:
	RainBufferShader(Camera* camera);
	~RainBufferShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instaceCount);

private:
	struct MatrixBuffer
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;

	Camera* m_camera;

	void setShaderParameters(ID3D11DeviceContext*, D3DXMATRIX, D3DXMATRIX, D3DXMATRIX);
	void renderShader(ID3D11DeviceContext*, int);
};

#endif