#ifndef _SHADER_H_
#define _SHADER_H_

#include <d3d11.h>
#include <d3dx10math.h>
#include <fstream>

class Shader
{
public:
	Shader() : m_techniqueIndex(0), m_deviceContext(0) {};
	virtual ~Shader() {};

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext) = 0;
	virtual void render(int indexCount, int instanceCount) = 0;
	virtual void setWorldMatrix(D3DXMATRIX matrix) {m_worldMatrix = matrix;};
	void setTechnique(int techniqueIndex) { m_techniqueIndex = techniqueIndex; };

protected:
	ID3D11DeviceContext* m_deviceContext;
	int					 m_techniqueIndex;
	D3DXMATRIX			 m_worldMatrix;
};

#endif