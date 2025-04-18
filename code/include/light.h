#ifndef _LIGHT_H_
#define _LIGHT_H_

#include <d3dx10math.h>

class Light
{
public:
	Light();
	~Light();

	void setAmbientColor(float red, float green, float blue, float alpha);
	void setDiffuseColor(float red, float green, float blue, float alpha);
	void setPosition(float x, float y, float z);
	void setLookAt(float x, float y, float z);
	void setSpecularColor(float red, float green, float blue, float alpha);
	void setSpecularPower(float power);

	D3DXVECTOR4 getAmbientColor();
	D3DXVECTOR4 getDiffuseColor();
	D3DXVECTOR3 getPosition();
	D3DXVECTOR4 getSpecularColor();
	float getSpecularPower();

	void generateViewMatrix();
	void generateProjectionMatrix(float nearPlane, float farPlane);

	void getViewMatrix(D3DXMATRIX& viewMatrix);
	void getProjectionMatrix(D3DXMATRIX& projectionMatrix);

private:
	D3DXVECTOR4 m_ambientColor;
	D3DXVECTOR4 m_diffuseColor;
	D3DXVECTOR3 m_position;
	D3DXVECTOR3 m_lookAt;
	D3DXMATRIX m_viewMatrix;
	D3DXMATRIX m_projectionMatrix;
	D3DXVECTOR4 m_specularColor;
	float m_specularPower;
};

#endif