#ifndef _PARTICLESHADER_H_
#define _PARTICLESHADER_H_

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

class Camera;

enum PSType
{
	DROP,
	SPLASH,
	POINT_LIGHT,
	TYPE_COUNT
};

class ParticleShader : public Shader
{
public:
	ParticleShader(Camera* camera);
	~ParticleShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

private:
	struct MatrixBuffer
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
		D3DXMATRIX lightView;
		D3DXMATRIX lightProjection;
	};

	Camera*					   m_camera;

	ID3D11VertexShader*        m_vertexShader;
	ID3D11PixelShader*         m_pixelShaders[TYPE_COUNT];
	ID3D11InputLayout*         m_layout;
	ID3D11Buffer*              m_matrixBuffer;
	ID3D11SamplerState*        m_sampleStateClamp;
	ID3D11DepthStencilState*   m_depthState;
};

#endif