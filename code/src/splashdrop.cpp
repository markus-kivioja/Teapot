#include "../include/splashdrop.h"
#include "../include/shaderrenderers/particleshader.h"
#include "../include/constant_parameters.h"
#include <d3dx10math.h>
#include <stdio.h>

SplashDrop::SplashDrop(ParticleShader* particleShader, D3DXVECTOR3 pos, D3DXVECTOR3 vel, int lifeSpan, float gravity) :
	m_shader(particleShader),
	m_position(pos),
	m_velocity(vel),
	m_lifeSpan(lifeSpan),
	m_gravity(gravity),
	m_lived(0),
	m_dead(false)
{
}

SplashDrop::~SplashDrop()
{
}

void SplashDrop::getPosition(float* position)
{
	position[0] = m_position.x;
	position[1] = m_position.y;
	position[2] = m_position.z;
}

void SplashDrop::update(int dt)
{
	m_velocity.y -= m_gravity * (float)dt / TARGET_FRAME_TIME;
	m_position += m_velocity * (float)dt / TARGET_FRAME_TIME;
	m_lived += dt;
	if (m_lived > m_lifeSpan)
		m_dead = true;
}

bool SplashDrop::isDead() const
{
	return m_dead;
}