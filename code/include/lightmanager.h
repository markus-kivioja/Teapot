#ifndef LIGHT_MANAGER
#define LIGHT_MANAGER

#include <d3d11.h>
#include <d3dx10math.h>
#include "constant_parameters.h"

class ParticleShader;

struct PointLight
{
	D3DXVECTOR4 position;
	D3DXVECTOR4 color;
};
struct SpotLight
{
	D3DXVECTOR4 positionSin;
	D3DXVECTOR4 directionCos;
	D3DXVECTOR4 colorRcpSin;
};

class LightManager
{
public:
	LightManager(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ParticleShader* particleShader);
	~LightManager();

	void update(int dt);
	void render(D3DXVECTOR3 cameraDirection);

	const PointLight* getPointLights() { return m_pointLights; };
	const SpotLight* getSpotLights() { return m_spotLights; };

private:
	struct VertexType
	{
		D3DXVECTOR3 position;
	    D3DXVECTOR2 texture;
	};
	struct InstanceType
	{
		D3DXVECTOR3 instancePos;
		D3DXVECTOR3 instanceColor;
	};

	ParticleShader*			m_particleShader;
	
	PointLight				m_pointLights[POINT_LIGHT_COUNT];
	SpotLight				m_spotLights[SPOT_LIGHT_COUNT];

	ID3D11Device*			m_device;
	ID3D11DeviceContext*	m_deviceContext;

	ID3D11Buffer*           m_vertexBuffer;
	ID3D11Buffer*           m_indexBuffer;
	ID3D11Buffer*			m_instanceBuffer;
};

#endif