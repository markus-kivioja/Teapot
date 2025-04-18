#ifndef _SSLRSHADER_H_
#define _SSLRSHADER_H_

#include "shader.h"

class Camera;

class SSLRShader : public Shader
{
public:
	SSLRShader(Camera* camera, int width, int height);
	virtual ~SSLRShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

private:
	Camera*							m_camera;
	ID3D11ComputeShader*			m_computeShader;
	int								m_width;
	int								m_height;

	ID3D11Buffer*					m_matrixBuffer;
};

#endif