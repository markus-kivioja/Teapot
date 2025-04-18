#ifndef _LIGHTINGSHADER_H_
#define _LIGHTINGSHADER_H_

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <vector>
#include <stdint.h>
#include "../constant_parameters.h"

class Camera;
struct PointLight;
struct SpotLight;

class LightingShader : public Shader
{
public:
	LightingShader(Camera* camera, const PointLight* pointLightBuffer, const SpotLight* spotLightBuffer, int width, int height, float nearPlane, float farPlane);
	virtual ~LightingShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

	void setDimensions(int width, int height);
	void transformLights();

private:
	struct MatrixBuffer
	{
		D3DXMATRIX cameraProjection;
		D3DXMATRIX inverseView;
		D3DXMATRIX inverseViewLightVP[SPOT_LIGHT_COUNT];
	};

	uint32_t						m_width;
	uint32_t						m_height;

	float							m_nearPlane;
	float							m_farPlane;

	Camera*							m_camera;

	D3DXVECTOR4*					m_normalBufferData;
	D3DXVECTOR4*					m_uvBufferData;

	ID3D11ComputeShader*			m_lightTransformShader;
	ID3D11ComputeShader*			m_computeShader;
	ID3D11InputLayout*				m_layout;
	ID3D11SamplerState*				m_sampleStateWrap;
	ID3D11SamplerState*				m_shadowSampler;
	
	ID3D11Buffer*					m_matrixBuffer;
	ID3D11Buffer*					m_viewMatrixBuffer;

	ID3D11Buffer*					m_wsPointLightBuffer;
	ID3D11Buffer*					m_vsPointLightBuffer;
	ID3D11Buffer*					m_wsSpotLightBuffer;
	ID3D11Buffer*					m_vsSpotLightBuffer;
	
	ID3D11ShaderResourceView*		m_wsPointLightSRV;
	ID3D11UnorderedAccessView*		m_vsPointLightUAV;
	ID3D11ShaderResourceView*		m_vsPointLightSRV;

	ID3D11ShaderResourceView*		m_wsSpotLightSRV;
	ID3D11UnorderedAccessView*		m_vsSpotLightUAV;
	ID3D11ShaderResourceView*		m_vsSpotLightSRV;

	const PointLight*				m_pointLightBuffer;
	const SpotLight*				m_spotLightBuffer;
};

#endif