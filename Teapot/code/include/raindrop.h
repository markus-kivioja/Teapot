#ifndef _RAINDROP_H_
#define _RAINDROP_H_
#include <d3d11.h>
#include <d3dx10math.h>

class ParticleShader;

class RainDrop
{
public:
	RainDrop(ParticleShader* particleShader, float, float, float, float, float, D3DXVECTOR3);
	~RainDrop();
	void update(int);
	void getDiePosition(D3DXVECTOR3&);
	void getDiePosNormal(D3DXVECTOR3&);
	bool isDead() const;

	void getPosition(float*);

private:
	float             m_x;
	float             m_y;
	float             m_z;
	float             m_speed;
	float             m_dieHeight;
	bool              m_dead;
	D3DXVECTOR3       m_diePosNormal;

	ParticleShader*   m_shader;
	D3DXMATRIX        m_transform;
};

#endif