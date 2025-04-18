#include "../include/light.h"

Light::Light()
{
}

Light::~Light()
{
}

void Light::setAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = D3DXVECTOR4(red, green, blue, alpha);
}

void Light::setDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = D3DXVECTOR4(red, green, blue, alpha);
}

void Light::setPosition(float x, float y, float z)
{
	m_position = D3DXVECTOR3(x, y, z);
}

void Light::setLookAt(float x, float y, float z)
{
	m_lookAt.x = x;
	m_lookAt.y = y;
	m_lookAt.z = z;
}

D3DXVECTOR4 Light::getAmbientColor()
{
	return m_ambientColor;
}

D3DXVECTOR4 Light::getDiffuseColor()
{
	return m_diffuseColor;
}

D3DXVECTOR3 Light::getPosition()
{
	return m_position;
}

void Light::generateViewMatrix()
{
	D3DXVECTOR3 up;

	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_viewMatrix, &m_position, &m_lookAt, &up);
}

void Light::generateProjectionMatrix(float nearPlane, float farPlane)
{
	float fieldOfView, screenAspect;

	fieldOfView = (float)D3DX_PI / 2.0f;
	screenAspect = 1.0f;

	//D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fieldOfView, screenAspect, nearPlane, farPlane);
	D3DXMatrixOrthoLH(&m_projectionMatrix, 15.0f, 15.0f, nearPlane, farPlane);
}

void Light::getViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void Light::getProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}

void Light::setSpecularColor(float red, float green, float blue, float alpha)
{
	m_specularColor = D3DXVECTOR4(red, green, blue, alpha);
}

void Light::setSpecularPower(float power)
{
	m_specularPower = power;
}

D3DXVECTOR4 Light::getSpecularColor()
{
	return m_specularColor;
}

float Light::getSpecularPower()
{
	return m_specularPower;
}

