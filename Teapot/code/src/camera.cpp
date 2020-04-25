#include "../include/camera.h"
#include "../include/funcs.h"

Camera::Camera(int screenWidth, int screenHeight, float nearPlane, float farPlane, float fov)
{
	float aspect = (float)screenWidth / (float)screenHeight;

	D3DXMatrixPerspectiveFovLH(&m_projectionMatrix, fov, aspect, nearPlane, farPlane);

	m_direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	m_position = D3DXVECTOR3(0.0f, 2.0f, -5.0f);
}

Camera::~Camera(void)
{
}

void Camera::move(float straight, float sideways, float upDown)
{
	D3DXVECTOR3 strafeVec(-m_direction.z, 0.0f, m_direction.x);
	m_position += straight * m_direction +
				  sideways * normalize(strafeVec) + 
				  upDown * normalize(cross(m_direction, strafeVec));

	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
	D3DXVECTOR3 lookAt(m_position + m_direction);
	D3DXMatrixLookAtLH(&m_viewMatrix, &m_position, &lookAt, &up);
}

void Camera::turn(float vertical, float horizontal)
{
	m_direction += vertical * D3DXVECTOR3(0.0f, 1.0f, 0.0f) + 
				   horizontal * normalize(D3DXVECTOR3(-m_direction.z, 0.0f, m_direction.x));
	
	m_direction = normalize(m_direction);
}

void Camera::getProjectionMatrix(D3DXMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}

void Camera::getViewMatrix(D3DXMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void Camera::getPosition(D3DXVECTOR3& position)
{
	position = m_position;
}
