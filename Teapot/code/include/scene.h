#ifndef _SCENE_H_
#define _SCENE_H_

#include <vector>

#define RAINBUF_LAYER_COUNT 2
#define RAINBUF_DEPTH_NORMAL 0
#define RAINBUF_UV 1

#define GBUF_LAYER_COUNT 2
#define GBUF_DEPTH_ALBEDO 0
#define GBUF_NORMAL 1

#define LUMINANCE_MIP_COUNT 7

class Renderer;
class Object;
class RainCaster;
class RenderTarget;

class RainBufferShader;
class ShadowMapShader;
class GBufferShader;
class LightingShader;
class SkyboxShader;
class BlitShader;
class SSLRShader;
class SSAOShader;
class BlurShader;
class CombineTonemapShader;
class DownsampleShader;

class Input;
class Camera;
class LightManager;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

class Scene
{
public:
	Scene(Input*);
	~Scene();

	bool init(int screenWidth, int screenHeight, HWND hwnd);

	void update(int dt);
	void updateCamera();

private:
	float						m_lightAzimuth;
	float						m_lightElevation;
	int							m_mouseX;
	int							m_mouseY;

	RainBufferShader*			m_rainBufferShader;
	LightingShader*				m_lightingShader;
	BlitShader*					m_blitShader;
	GBufferShader*				m_gBufferShader;
	ShadowMapShader*			m_shadowMapShader;
	SkyboxShader*				m_skyboxShader;
	SSLRShader*					m_sslrShader;
	SSAOShader*					m_ssaoShader;
	BlurShader*				m_blurShader;
	CombineTonemapShader*		m_combineTonemapShader;
	DownsampleShader*			m_downsampleShader;

	RainCaster*					m_rainCaster;
	Renderer*					m_renderer;
	std::vector<Object*>		m_objects;
	Camera*						m_camera;
	Input*						m_input;
	LightManager*				m_lightManager;

	ID3D11Device*				m_device;
	ID3D11DeviceContext*		m_deviceContext;

	ID3D11ShaderResourceView*	m_cubeMap;

	RenderTarget*				m_rainBuffers[RAINBUF_LAYER_COUNT];
	RenderTarget*				m_spotShadowMaps;
	RenderTarget*				m_gBuffers[GBUF_LAYER_COUNT];
	RenderTarget*				m_lightingTarget;
	RenderTarget*				m_sslrTarget;
	RenderTarget*				m_ssaoTargets[2];
	RenderTarget*				m_particleTarget;
	RenderTarget*				m_bloomTargets[2];
	std::vector<RenderTarget*>	m_luminanceChain;

	void initRenderTargets(int screenWidth, int screenHeight);
	void initObjects();
	bool initShaders(float nearPlane, float farPlane);

	void renderRainBuffer();
	void renderUVBuffer();

	void renderScene();

	void renderShadowMaps();
	void renderGBuffers();
	void renderLighting();
	void renderSky();
	void renderParticles();
	void renderPostEffects();
	void renderFinalCombine();
};

#endif