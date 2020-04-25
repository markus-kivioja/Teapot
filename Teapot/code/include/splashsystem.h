#ifndef _SPLASHSYSTEM_H_

#include <vector>
#include <d3dx11async.h>
#include <d3dx10math.h>

class SplashDrop;
class ParticleShader;

class SplashSystem
{
public:
	SplashSystem(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ParticleShader* rainShader);
	~SplashSystem();

	void update(int dt);
	void render(D3DXVECTOR3 cameraDirection);
	void splash(D3DXVECTOR3 pos, D3DXVECTOR3 dir);
	
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

	std::vector<SplashDrop*>   m_drops;
	ParticleShader*            m_shader;
	ID3D11Device*              m_device;
	ID3D11DeviceContext*	   m_deviceContext;

	ID3D11Buffer*              m_vertexBuffer;
	ID3D11Buffer*              m_indexBuffer;
	ID3D11Buffer*			   m_instanceBuffer;
};

#endif