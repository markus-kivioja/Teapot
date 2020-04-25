#pragma once

#include <d3dx10math.h>

class Camera
{
public:
	Camera(int screenWidth, int screenHeight, float nearPlane, float farPlane, float fov);
	~Camera();

	void getProjectionMatrix(D3DXMATRIX& projectionMatrix);
	void getViewMatrix(D3DXMATRIX& viewMatrix);
	void getPosition(D3DXVECTOR3& position);

	void move(float straight, float sideways, float upDown);
	void turn(float vertical, float horizontal);

	D3DXVECTOR3 getDirection() { return m_direction; };

private:
	D3DXVECTOR3 m_position;
	D3DXVECTOR3 m_direction;

	D3DXMATRIX m_projectionMatrix;
	D3DXMATRIX m_viewMatrix;
};

