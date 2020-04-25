#include "../include/raindrop.h"
#include "../include/shaderrenderers/particleshader.h"
#include "../include/constant_parameters.h"
#include <d3dx10math.h>
#include <stdio.h>
#include <math.h>

RainDrop::RainDrop(ParticleShader* shader, float x, float y, float z, float speed, float dieHeight, D3DXVECTOR3 diePosNormal) :
	m_shader(shader),
	m_x(x),
	m_y(y),
	m_z(z),
	m_speed(speed),
	m_dieHeight(dieHeight),
	m_dead(false),
	m_diePosNormal(diePosNormal)
{
}

RainDrop::~RainDrop()
{
}

void RainDrop::getPosition(float* position)
{
	position[0] = m_x;
	position[1] = m_y;
	position[2] = m_z;
}

void RainDrop::update(int dt)
{
	m_y -= m_speed * dt / TARGET_FRAME_TIME;
	if (m_y < m_dieHeight)
		m_dead = true;
}

bool RainDrop::isDead() const
{
	return m_dead;
}

void RainDrop::getDiePosition(D3DXVECTOR3& diePosition)
{
	diePosition = D3DXVECTOR3(m_x, m_dieHeight, m_z);
}

void RainDrop::getDiePosNormal(D3DXVECTOR3& diePosNormal)
{
	diePosNormal = m_diePosNormal;
}
