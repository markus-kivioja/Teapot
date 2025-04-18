#ifndef _SSAOSHADER_H_
#define _SSAOSHADER_H_

#include "shader.h"

#define KERNEL_SIZE 16
#define ROTATION_TEX_DIM 4
#define ROTATION_COUNT (ROTATION_TEX_DIM * ROTATION_TEX_DIM)

class Camera;

class SSAOShader : public Shader
{
public:
	SSAOShader(Camera* camera, int width, int height);
	virtual ~SSAOShader();

	virtual bool init(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void render(int indexCount, int instanceCount);

private:
	Camera*							m_camera;
	ID3D11ComputeShader*			m_computeShader;
	int								m_width;
	int								m_height;

	ID3D11Buffer*					m_matrixBuffer;
	ID3D11Buffer*					m_kernelBuffer;

	ID3D11Texture2D*				m_randomRotationsTexture;
	ID3D11ShaderResourceView*		m_randomRotations;
};

#endif