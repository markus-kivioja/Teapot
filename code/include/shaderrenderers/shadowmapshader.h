#ifndef _SHADOWMAPSHADER_H_
#define _SHADOWMAPSHADER_H_

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

struct SpotLight;

class ShadowMapShader : public Shader
{
public:
	ShadowMapShader(float nearPlane, float farPlane);
	~ShadowMapShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

	void setSpotLight(const SpotLight* spotLight) { m_spotLight = spotLight; };

private:
	struct MatrixBuffer
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	const SpotLight*		m_spotLight;

	ID3D11VertexShader*		m_vertexShader;
	ID3D11InputLayout*		m_layout;
	ID3D11Buffer*			m_matrixBuffer;
	float					m_nearPlane;
	float					m_farPlane;
};

#endif