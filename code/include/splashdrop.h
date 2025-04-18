#ifndef _SPLASHDROP_H_
#define _SPLASHDROP_H_
#include <d3d11.h>
#include <d3dx10math.h>

class ParticleShader;

class SplashDrop
{
public:
	SplashDrop(ParticleShader* particleShader, D3DXVECTOR3, D3DXVECTOR3, int, float);
	~SplashDrop();
	void update(int dt);
	bool isDead() const;

	void getPosition(float*);

private:
	D3DXVECTOR3       m_position;
	D3DXVECTOR3       m_velocity;
	int               m_lifeSpan;
	float             m_gravity;
	int               m_lived;
	bool              m_dead;

	ParticleShader*   m_shader;
	D3DXMATRIX        m_transform;
};

#endif