#ifndef _RAINCASTER_H_
#define _RAINCASTER_H_

#include <vector>
#include <d3dx11async.h>
#include <d3dx10math.h>

class SplashSystem;
class GBufferShader;
class ParticleShader;
class RainDrop;

class RainCaster
{
public:
	RainCaster(ID3D11Device* device, ID3D11DeviceContext* deviceContext, D3DXVECTOR4* nData,
		GBufferShader* gBufShader, ParticleShader* rainShader);
	~RainCaster();

	bool init();

	void update(int dt);
	void render(D3DXVECTOR3 cameraDirection);
	
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

	std::vector<RainDrop*>   m_drops;
	ParticleShader*          m_shader;
	GBufferShader*           m_gBufferShader;
	SplashSystem*            m_splashSystem;
	D3DXVECTOR4*             m_normalBufferData;
	ID3D11Device*            m_device;
	ID3D11DeviceContext*	 m_deviceContext;

	ID3D11Buffer*            m_vertexBuffer;
	ID3D11Buffer*            m_indexBuffer;
	ID3D11Buffer*			 m_instanceBuffer;
};

#endif