#pragma once

#include "shader.h"
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <vector>

class Camera;

#define GROUND_HEIGHT 1.01f

enum PixelTechnique
{
	PUDDLE_TECHNIQUE,
	DRIBBLE_TECHNIQUE,
	TECHNIQUE_COUNT
};

class GBufferShader : public Shader
{
public:
	GBufferShader(Camera* camera, D3DXVECTOR4* normalBufferData, D3DXVECTOR4* uvBufferData);
	~GBufferShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

	void addWaveEffect(D3DXVECTOR3 pos);

	void addDribbleEffect(D3DXVECTOR3 pos, D3DXVECTOR3 dir);
	void updateEffects(int dt);

	void setShininess(float shininess) { m_shininess = shininess; };

	void addPuddlePosSize(D3DXVECTOR4 puddlePosSize) { m_puddlePosSizes.push_back(puddlePosSize); };

	const D3DXVECTOR4* getPuddlePosSizes() { return &m_puddlePosSizes[0]; };

private:
	struct VSConstantBuffer
	{
		D3DXMATRIX world;
		D3DXMATRIX view;
		D3DXMATRIX projection;
	};

	struct PSConstantBuffer
	{
		D3DXVECTOR4 shininess;
		D3DXMATRIX view;
	};

	struct PuddleBuffer
	{
		D3DXVECTOR4 puddlePosSizes[5];
	};

	std::vector<D3DXVECTOR4>	m_waveEffects;
	std::vector<D3DXVECTOR4>	m_dribbleEffects;
	std::vector<D3DXVECTOR3>	m_dribbleDirs;

	D3DXVECTOR4*				m_normalBufferData;
	D3DXVECTOR4*				m_uvBufferData;

	ID3D11VertexShader*			m_vertexShader;
	ID3D11PixelShader*			m_pixelShaders[TECHNIQUE_COUNT];
	ID3D11InputLayout*			m_layout;
	ID3D11SamplerState*			m_sampleStateWrap;
	ID3D11Buffer*				m_vsConstantBuffer;
	ID3D11Buffer*				m_psConstantBuffer;
	ID3D11Buffer*				m_puddleBuffer;
	ID3D11Buffer*				m_waveBuffer;
	ID3D11Buffer*				m_dribbleBuffer;
	
	std::vector<D3DXVECTOR4>	m_puddlePosSizes;


	ID3D11ShaderResourceView*	m_puddleMask;

	Camera*						m_camera;

	float						m_shininess;
};

