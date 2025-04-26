#include "../include/renderer.h"
#include "../include/object.h"
#include "../include/light.h"
#include "../include/rendertarget.h"
#include "../include/lightmanager.h"
#include "../include/shaderrenderers/shadowmapshader.h"
#include "../include/shaderrenderers/lightingshader.h"
#include "../include/shaderrenderers/particleshader.h"
#include "../include/shaderrenderers/rainbuffershader.h"
#include "../include/shaderrenderers/blitshader.h"
#include "../include/shaderrenderers/gbuffershader.h"
#include "../include/shaderrenderers/skyboxshader.h"
#include "../include/shaderrenderers/sslrshader.h"
#include "../include/shaderrenderers/ssaoshader.h"
#include "../include/shaderrenderers/blurshader.h"
#include "../include/shaderrenderers/combinetonemapshader.h"
#include "../include/shaderrenderers/downsampleshader.h"
#include "../include/scene.h"
#include "../include/raincaster.h"
#include "../include/constant_parameters.h"
#include "../include/input.h"
#include "../include/camera.h"
#include "../include/funcs.h"
#include <math.h>

Scene::Scene(Input* input) :
	m_mouseX(-1),
	m_mouseY(-1),
	m_lightAzimuth(-4.112f),
	m_lightElevation(-0.922f),
	m_input(input),
	m_camera(0),
	m_lightingShader(0),
	m_blitShader(0),
	m_rainBufferShader(0),
	m_gBufferShader(0),
	m_shadowMapShader(0),
	m_skyboxShader(0),
	m_spotShadowMaps(0),
	m_renderer(0),
	m_rainCaster(0),
	m_lightingTarget(0),
	m_lightManager(0),
	m_cubeMap(0)
{
	for (int i = 0; i < GBUF_LAYER_COUNT; ++i)
		m_gBuffers[i] = 0;
	for (int i = 0; i < RAINBUF_LAYER_COUNT; ++i)
		m_rainBuffers[i] = 0;
}

Scene::~Scene()
{
	if (m_lightingShader)
		delete m_lightingShader;
	if (m_shadowMapShader)
		delete m_shadowMapShader;
	if (m_rainBufferShader)
		delete m_rainBufferShader;
	if (m_blitShader)
		delete m_blitShader;
	if (m_gBufferShader)
		delete m_gBufferShader;
	if (m_skyboxShader)
		delete m_skyboxShader;
	if (m_spotShadowMaps)
		delete m_spotShadowMaps;
	if (m_rainCaster)
		delete m_rainCaster;
	if (m_renderer)
		delete m_renderer;
	if (m_lightingTarget)
		delete m_lightingTarget;

	m_lightingShader = 0;
	m_shadowMapShader = 0;
	m_spotShadowMaps = 0;
	m_renderer = 0;
	m_rainCaster = 0;
	m_rainBufferShader = 0;

	for (unsigned int i = 0; i < m_objects.size(); ++i)
		delete m_objects[i];

	for (int i = 0; i < GBUF_LAYER_COUNT; ++i)
	{
		delete m_gBuffers[i];
		m_gBuffers[i] = 0;
	}
	for (int i = 0; i < RAINBUF_LAYER_COUNT; ++i)
	{
		delete m_rainBuffers[i];
		m_rainBuffers[i] = 0;
	}

	m_objects.clear();
}

bool Scene::init(int screenWidth, int screenHeight, HWND hwnd)
{
	// Reverse depth to improve precision with floating point z-buffer
#ifdef REVERSE_DEPTH
	float nearPlane = 100.0f;
	float farPlane = 0.1f;
#else
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
#endif
	
	m_renderer = new Renderer(screenWidth, screenHeight, hwnd);
	m_camera = new Camera(screenWidth, screenHeight, nearPlane, farPlane, (float)D3DX_PI / 4.0f);
	m_device = m_renderer->getDevice();
	m_deviceContext = m_renderer->getDeviceContext();

	ID3D11Texture2D* texture;
	D3DX11_IMAGE_LOAD_INFO loadInfo;
	loadInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	HRESULT result;
	D3DX11CreateTextureFromFile(m_device, "textures/cubemap.dds", &loadInfo, NULL, (ID3D11Resource**)&texture, &result);
	if(FAILED(result))
	{
		output("LOADING CUBE MAP FAILED\n");
		return false;
	}	
	D3D11_TEXTURE2D_DESC textureDesc;
	texture->GetDesc(&textureDesc);
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = textureDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;
	result = m_device->CreateShaderResourceView(texture, &srvDesc, &m_cubeMap);

	initRenderTargets(screenWidth, screenHeight);
	initObjects();
	return initShaders(nearPlane, farPlane);
}

void Scene::initRenderTargets(int screenWidth, int screenHeight)
{
	m_spotShadowMaps = new RenderTarget(m_device, m_deviceContext, BUFS_WIDTH, BUFS_HEIGHT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT_S8X24_UINT, 0, SPOT_LIGHT_COUNT);
	for (int i = 0; i < GBUF_LAYER_COUNT; ++i)
		m_gBuffers[i] = new RenderTarget(m_device, m_deviceContext, screenWidth, screenHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, !i ? DXGI_FORMAT_D32_FLOAT_S8X24_UINT : DXGI_FORMAT_UNKNOWN, 0);
	for (int i = 0; i < RAINBUF_LAYER_COUNT; ++i)
		m_rainBuffers[i] = new RenderTarget(m_device, m_deviceContext, BUFS_WIDTH, BUFS_HEIGHT, DXGI_FORMAT_R32G32B32A32_FLOAT, !i ? DXGI_FORMAT_D32_FLOAT_S8X24_UINT : DXGI_FORMAT_UNKNOWN, RT_FLAG_CPU_READ);

	m_lightingTarget = new RenderTarget(m_device, m_deviceContext, screenWidth, screenHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, RT_FLAG_UAV);

	m_sslrTarget = new RenderTarget(m_device, m_deviceContext, screenWidth, screenHeight, DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN, RT_FLAG_UAV);

	m_ssaoTargets[0] = new RenderTarget(m_device, m_deviceContext, screenWidth, screenHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, RT_FLAG_UAV);
	m_ssaoTargets[1] = new RenderTarget(m_device, m_deviceContext, screenWidth, screenHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN, RT_FLAG_UAV);

	m_particleTarget = new RenderTarget(m_device, m_deviceContext, screenWidth, screenHeight, DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN, RT_FLAG_UAV);

	int width = (int)(screenWidth * 0.5f), height = (int)(screenHeight * 0.5f);
	m_bloomTargets[0] = new RenderTarget(m_device, m_deviceContext, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN, 0);
	m_bloomTargets[1] = new RenderTarget(m_device, m_deviceContext, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN, 0);

	while (width > 1 && height > 1)
	{
		m_luminanceChain.push_back(new RenderTarget(m_device, m_deviceContext, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN, 0));
		width = (int)(width * 0.25f);
		height = (int)(height * 0.25f);
	}
	m_luminanceChain.push_back(new RenderTarget(m_device, m_deviceContext, 2, 2, DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN, 0));
	m_luminanceChain.push_back(new RenderTarget(m_device, m_deviceContext, 1, 1, DXGI_FORMAT_R16G16B16A16_FLOAT,  DXGI_FORMAT_UNKNOWN, 0));
}

void Scene::initObjects()
{
#ifdef BINARY_MESH
	Object* object = new Object(m_device, m_deviceContext, "models/table.mymesh", "textures/ground_colorMap.dds", 
								"textures/ground_normalMap.dds", "textures/ground_displacementMap.dds", 
								"textures/dummy_specularMap.dds", false, false);
#else
	Object* object = new Object(m_device, m_deviceContext, "models/table.obj", "textures/ground_colorMap.dds", 
								"textures/ground_normalMap.dds", "textures/ground_displacementMap.dds", 
								"textures/dummy_specularMap.dds", false, false);
#endif
	object->setShininess(49.0f);
	object->setPosition(0.0f, 1.0f, 0.0f);
	m_objects.push_back(object);

#ifdef BINARY_MESH
	object = new Object(m_device, m_deviceContext, "models/newTeapot.mymesh", "textures/porcelain.dds",
						"textures/dummy_normalMap.dds", "textures/dummy_displacementMap.dds", 
						"textures/dummy_specularMap.dds", true, false);
#else
	object = new Object(m_device, m_deviceContext, "models/newTeapot.obj", "textures/porcelain.dds",
						"textures/dummy_normalMap.dds", "textures/dummy_displacementMap.dds", 
						"textures/dummy_specularMap.dds", true, false);
#endif
	object->setBackfaceCulling(false);
	object->setShininess(110.0f);
	object->setPosition(0.0f, 0.95f, 0.0f);
	object->setScale(0.5f);
	m_objects.push_back(object);

#ifdef BINARY_MESH
	object = new Object(m_device, m_deviceContext, "models/mickey.mymesh", "textures/mickey.dds",
						"textures/dummy_normalMap.dds", "textures/dummy_displacementMap.dds", 
						"textures/dummy_specularMap.dds", true, false);
#else
	object = new Object(m_device, m_deviceContext, "models/mickey.obj", "textures/mickey.dds",
						"textures/dummy_normalMap.dds", "textures/dummy_displacementMap.dds", 
						"textures/dummy_specularMap.dds", true, false);
#endif
	object->setShininess(49.0f);
	object->setPosition(-2.5f, 1.2f, -2.0f);
	object->setScale(0.2f);
	object->setRotation( -(float)D3DX_PI / 10.0f, 0.0f, -(float)D3DX_PI / 1.8f);
	m_objects.push_back(object);

#ifdef BINARY_MESH
	object = new Object(m_device, m_deviceContext, "models/rock.mymesh", "textures/rock_color.dds",
						"textures/rock_normalMap.dds", "textures/rock_displacementMap.dds", 
						"textures/rock_specularMap.dds", true, true);
#else
	object = new Object(m_device, m_deviceContext, "models/rock.obj", "textures/rock_color.dds",
						"textures/rock_normalMap.dds", "textures/rock_displacementMap.dds", 
						"textures/rock_specularMap.dds", true, true);
#endif
	object->setShininess(9.0f);
	object->setPosition(2.0f, 1.7f, 2.0f);
	object->setScale(0.04f);
	object->setRotation( -(float)D3DX_PI / 2.0f, 0.0f, 0.0f);
	m_objects.push_back(object);
}

bool Scene::initShaders(float nearPlane, float farPlane)
{
	m_rainBufferShader = new RainBufferShader(m_camera);
	if (!m_rainBufferShader->init(m_device, m_deviceContext))
		return false;

	renderRainBuffer();	
	void* nData;
	m_rainBuffers[RAINBUF_DEPTH_NORMAL]->getData(&nData);
	
	void* uvData;
	m_rainBuffers[RAINBUF_UV]->getData(&uvData);

	m_shadowMapShader = new ShadowMapShader(nearPlane, farPlane);
	if (!m_shadowMapShader->init(m_device, m_deviceContext))
		return false;

	m_blitShader = new BlitShader();
	if (!m_blitShader->init(m_device, m_deviceContext))
		return false;

	m_gBufferShader = new GBufferShader(m_camera, (D3DXVECTOR4*)nData, (D3DXVECTOR4*)uvData);
	m_gBufferShader->addPuddlePosSize(D3DXVECTOR4(-4.2f, -2.7f, 4.8f, 3.2f));
	m_gBufferShader->addPuddlePosSize(D3DXVECTOR4(0.8f, -3.0f, 3.0f, 4.0f));
	m_gBufferShader->addPuddlePosSize(D3DXVECTOR4(-4.2f, 0.7f, 4.8f, 3.2f));
	m_gBufferShader->addPuddlePosSize(D3DXVECTOR4(0.8f, 1.0f, 3.0f, 4.0f));
	if (!m_gBufferShader->init(m_device, m_deviceContext))
		return false;

	m_skyboxShader = new SkyboxShader(m_camera);
	if (!m_skyboxShader->init(m_device, m_deviceContext))
		return false;

	ParticleShader* particleShader = new ParticleShader(m_camera);
	if (!particleShader->init(m_device, m_deviceContext))
		return false;

	m_lightManager = new LightManager(m_device, m_deviceContext, particleShader);

	m_lightingShader = new LightingShader(m_camera, m_lightManager->getPointLights(), m_lightManager->getSpotLights(),
										  m_lightingTarget->getWidth(), m_lightingTarget->getHeight(), nearPlane, farPlane);
	if (!m_lightingShader->init(m_device, m_deviceContext))
		return false;

	m_sslrShader = new SSLRShader(m_camera, m_lightingTarget->getWidth(), m_lightingTarget->getHeight());
	if (!m_sslrShader->init(m_device, m_deviceContext))
		return false;

	m_ssaoShader = new SSAOShader(m_camera, m_lightingTarget->getWidth(), m_lightingTarget->getHeight());
	if (!m_ssaoShader->init(m_device, m_deviceContext))
		return false;

	m_blurShader = new BlurShader();
	if (!m_blurShader->init(m_device, m_deviceContext))
		return false;

	m_combineTonemapShader = new CombineTonemapShader();
	if (!m_combineTonemapShader->init(m_device, m_deviceContext))
		return false;

	m_downsampleShader = new DownsampleShader();
	if (!m_downsampleShader->init(m_device, m_deviceContext))
		return false;

	m_rainCaster = new RainCaster(m_device, m_deviceContext, (D3DXVECTOR4*)nData, m_gBufferShader, particleShader);
	return m_rainCaster->init();
}

bool updateOn = true;
void Scene::updateCamera()
{
	int x, y;
	m_input->getMouseLocation(x, y);

	float straight = 0.0f;
	float sideways = 0.0f;
	float upDown = 0.0f;

	if (m_input->isCtrlPressed())
	{
		if (m_input->isWPressed())
			upDown = -0.01f;
		else if (m_input->isSPressed())
			upDown = 0.01f;
	}
	else
	{
		if (m_input->isWPressed())
			straight = 0.04f;
		else if (m_input->isSPressed())
			straight = -0.04f;
	}

	if (m_input->isDPressed())
		sideways = -0.01f;
	else if (m_input->isAPressed())
		sideways = 0.01f;

	m_camera->move(straight, sideways, upDown);

	if (m_input->isSpacePressed())
		updateOn = false;
	else
		updateOn = true;

	static constexpr float mouseSensitivity = 0.0005f;
	if (m_mouseX != -1 && (m_mouseX != x || m_mouseY != y))
		m_camera->turn((m_mouseY - y) * mouseSensitivity, (m_mouseX - x) * mouseSensitivity);

	
	m_mouseX = x;
	m_mouseY = y;
}

void Scene::update(int dt)
{
	//output("FPS: %f\n", 1000.0f / (float)dt);

	updateCamera();
	if (updateOn)
	{
		m_rainCaster->update(dt);
		m_gBufferShader->updateEffects(dt);
		m_lightManager->update(dt);
	}

	renderScene();
}

void Scene::renderScene()
{
	renderShadowMaps();
	renderGBuffers();
	renderLighting();
	renderSky();
	renderParticles();
	renderPostEffects();
	renderFinalCombine();

	m_renderer->present();
}

void Scene::renderRainBuffer()
{
	for (int i = 0; i < RAINBUF_LAYER_COUNT; ++i)
		m_rainBuffers[i]->clear(D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
	m_renderer->setRenderTargets(m_rainBuffers, RAINBUF_LAYER_COUNT);
	m_renderer->setDepthCompareLess();

	D3DXMATRIX modelMatrix;
	for (unsigned int i = 0; i < m_objects.size(); ++i)
	{
		bool disableCull = !m_objects[i]->cullBackface();
		if (disableCull)
			m_renderer->disableCull();
		m_objects[i]->render(m_rainBufferShader);
		if (disableCull)
			m_renderer->enableCull();
	}

	m_renderer->setBackBufferRenderTarget();
	m_renderer->resetViewport();
	m_renderer->enableDepth();
}

void Scene::renderShadowMaps()
{
#ifdef REVERSE_DEPTH
	m_spotShadowMaps->clear(D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f), 0.0f);
#else
	m_spotShadowMaps->clear(D3DXVECTOR4(1.0f, 0.0f, 0.0f, 1.0f), 1.0f);
#endif

	const SpotLight* spotLights = m_lightManager->getSpotLights();

	for (int i = 0; i < SPOT_LIGHT_COUNT; ++i)
	{
		m_shadowMapShader->setSpotLight(&spotLights[i]);
		m_spotShadowMaps->setActiveDepthSlice(i);
		m_renderer->setRenderTargets(&m_spotShadowMaps, 1);
		for (unsigned int j = 0; j < m_objects.size(); ++j)
		{
			bool disableCull = !m_objects[i]->cullBackface();
			if (disableCull)
				m_renderer->disableCull();
			m_objects[j]->render(m_shadowMapShader);
			if (disableCull)
				m_renderer->enableCull();
		}
	}

	m_renderer->setBackBufferRenderTarget();
	m_renderer->resetViewport();
}

void Scene::renderGBuffers()
{
	for (int i = 0; i < GBUF_LAYER_COUNT; ++i)
#ifdef REVERSE_DEPTH
		m_gBuffers[i]->clear(D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 0.0f);
#else
		m_gBuffers[i]->clear(D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f), 1.0f);
#endif

	m_renderer->setRenderTargets(m_gBuffers, GBUF_LAYER_COUNT);
	
	m_gBufferShader->setTechnique(DRIBBLE_TECHNIQUE);
	for (unsigned int i = 1; i < m_objects.size(); ++i)
	{
		bool disableCull = !m_objects[i]->cullBackface();
		if (disableCull)
			m_renderer->disableCull();
		m_gBufferShader->setShininess(m_objects[i]->getShininess());
		m_objects[i]->render(m_gBufferShader);
		if (disableCull)
			m_renderer->enableCull();
	}

	m_gBufferShader->setTechnique(PUDDLE_TECHNIQUE);
	m_objects[0]->render(m_gBufferShader);

	ID3D11ShaderResourceView* temp[2];
	temp[0] = 0;
	temp[1] = 0;
	m_deviceContext->PSSetShaderResources(0, 2, temp);
}

void Scene::renderLighting()
{
	RenderTarget* nullTargets[] = {0, 0};
	m_renderer->setRenderTargets(nullTargets, GBUF_LAYER_COUNT);

	m_lightingShader->transformLights();

	ID3D11UnorderedAccessView* uavs[] = {m_lightingTarget->getUnorderedAccessView(), NULL};
	m_deviceContext->CSSetUnorderedAccessViews(0, 2, uavs, 0);

	ID3D11ShaderResourceView* depth = m_gBuffers[GBUF_DEPTH_ALBEDO]->getDepthShaderResourceView();
	ID3D11ShaderResourceView* color = m_gBuffers[GBUF_DEPTH_ALBEDO]->getShaderResourceView();
	ID3D11ShaderResourceView* normal = m_gBuffers[GBUF_NORMAL]->getShaderResourceView();
	ID3D11ShaderResourceView* gBufSRVs[] = {depth, color, normal};
	ID3D11ShaderResourceView* shadowMap = m_spotShadowMaps->getDepthShaderResourceView();
	m_deviceContext->CSSetShaderResources(0, 3, gBufSRVs);
	m_deviceContext->CSSetShaderResources(3, 1, &shadowMap);
	m_deviceContext->CSSetShaderResources(6, 1, &m_cubeMap);

	m_lightingShader->render(0, 0);

	ID3D11UnorderedAccessView* temp[] = {0};
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, temp, 0);
	ID3D11ShaderResourceView* temp2[] = {0, 0, 0, 0, 0, 0};
	m_deviceContext->CSSetShaderResources(0, 6, temp2);

	//ID3D11ShaderResourceView* srv = m_lightingTarget->getShaderResourceView();
	//m_deviceContext->PSSetShaderResources(0, 1, &srv);
	//m_blitShader->setPositionSize(D3DXVECTOR4(-1, -1, 2, 2));
	//m_renderer->disableDepth();
	//m_blitShader->setTechnique(NORMAL_TECHNIQUE);
	//m_blitShader->render(4, 1);
	//m_renderer->enableDepth();

	m_deviceContext->PSSetShaderResources(0, 2, temp2);
}

void Scene::renderSky()
{
	m_renderer->setRenderAndDepthTargets(&m_lightingTarget, m_gBuffers[GBUF_DEPTH_ALBEDO], 1);
	m_deviceContext->PSSetShaderResources(0, 1, &m_cubeMap);
	m_skyboxShader->render(0, 1);
}

void Scene::renderParticles()
{
	m_renderer->enableBlending();
	
	m_particleTarget->clear(D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f);
	m_renderer->setRenderAndDepthTargets(&m_lightingTarget, m_gBuffers[GBUF_DEPTH_ALBEDO], 1);
	m_rainCaster->render(m_camera->getDirection());


	// TODO: Light particles

	m_lightManager->render(m_camera->getDirection());

	m_renderer->enableDepth();
	m_renderer->disableBlending();

	ID3D11ShaderResourceView* temp[] = {0};
	m_deviceContext->PSSetShaderResources(0, 1, temp);
}

void Scene::renderPostEffects()
{
	m_renderer->setDepthTarget(NULL);

	// SSLR
	m_sslrTarget->clear(D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f), 0.0f);
	ID3D11UnorderedAccessView* sslrUAV = m_sslrTarget->getUnorderedAccessView();
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &sslrUAV, 0);
	ID3D11ShaderResourceView* depth = m_gBuffers[GBUF_DEPTH_ALBEDO]->getDepthShaderResourceView();
	ID3D11ShaderResourceView* lighting = m_lightingTarget->getShaderResourceView();
	ID3D11ShaderResourceView* normal = m_gBuffers[GBUF_NORMAL]->getShaderResourceView();
	ID3D11ShaderResourceView* sslrSrvs[] = {depth, lighting, normal};
	m_deviceContext->CSSetShaderResources(0, 3, sslrSrvs);
	m_sslrShader->render(0, 0);

	// SSAO
	m_ssaoTargets[0]->clear(D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
	ID3D11UnorderedAccessView* ssaoUAV = m_ssaoTargets[0]->getUnorderedAccessView();
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, &ssaoUAV, 0);
	ID3D11ShaderResourceView* ssaoSrvs[] = {depth, normal};
	m_deviceContext->CSSetShaderResources(0, 2, ssaoSrvs);
	m_ssaoShader->render(0, 0);
	ID3D11UnorderedAccessView* nullUAV[1] = {NULL};
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Blur SSAO x-direction
	D3DXVECTOR4 pixelSize = D3DXVECTOR4(1.0f / m_ssaoTargets[0]->getWidth(),
										1.0f / m_ssaoTargets[0]->getHeight(),
										0, 0);
	m_blurShader->setPixelSize(pixelSize);
	m_renderer->setRenderTargets(&m_ssaoTargets[1], 1);
	ID3D11ShaderResourceView* blurSrv = m_ssaoTargets[0]->getShaderResourceView();
	m_deviceContext->PSSetShaderResources(0, 1, &blurSrv);
	m_blurShader->setSigma(2.5f);
	m_blurShader->setLuminanceThreshold(0.0f);
	m_blurShader->setTechnique(X_TECHNIQUE);
	m_blurShader->render(4, 0);
	ID3D11ShaderResourceView* nullSRV[1] = {NULL};
	m_deviceContext->PSSetShaderResources(0, 1, nullSRV);

	// Blur SSAO y-direction
	m_renderer->setRenderTargets(&m_ssaoTargets[0], 1);
	blurSrv = m_ssaoTargets[1]->getShaderResourceView();
	m_deviceContext->PSSetShaderResources(0, 1, &blurSrv);
	m_blurShader->setTechnique(Y_TECHNIQUE);
	m_blurShader->render(4, 0);

	// Luminance mip chain
	// First do 2x2 downsample with logaritmic luminance calculation
	m_renderer->setRenderTargets(&m_luminanceChain[0], 1);
	m_deviceContext->PSSetShaderResources(0, 1, &lighting);
	m_downsampleShader->setTechnique(TECHNIQUE_2x2);
	pixelSize = D3DXVECTOR4(1.0f / m_lightingTarget->getWidth(), 
										1.0f / m_lightingTarget->getHeight(), 0, 0);
	m_downsampleShader->setPixelSize(pixelSize);
	m_downsampleShader->render(4, 0);

	// Then do 4x4 downsample until we have 1x1 pixels
	m_downsampleShader->setTechnique(TECHNIQUE_4x4);
	uint32_t chainSize = m_luminanceChain.size();
	for (uint32_t i = 1; i < chainSize; ++i)
	{
		m_renderer->setRenderTargets(&m_luminanceChain[i], 1);
		ID3D11ShaderResourceView* source = m_luminanceChain[i - 1]->getShaderResourceView();
		m_deviceContext->PSSetShaderResources(0, 1, &source);
		pixelSize = D3DXVECTOR4(1.0f / (float)m_luminanceChain[i - 1]->getWidth(), 
											1.0f / (float)m_luminanceChain[i - 1]->getHeight(), 0, 0);
		m_downsampleShader->setPixelSize(pixelSize);
		m_downsampleShader->render(4, 0);
	}

	// Bloom
	pixelSize = D3DXVECTOR4(1.0f / m_bloomTargets[0]->getWidth(),
							1.0f / m_bloomTargets[0]->getHeight(),
							0, 0);
	m_blurShader->setPixelSize(pixelSize);
	m_renderer->setRenderTargets(&m_bloomTargets[0], 1);
	ID3D11ShaderResourceView* bloomSource = m_luminanceChain[0]->getShaderResourceView();
	m_deviceContext->PSSetShaderResources(0, 1, &bloomSource);
	m_blurShader->setSigma(5.5f);
	m_blurShader->setLuminanceThreshold(0.9f);
	m_blurShader->setTechnique(X_TECHNIQUE);
	m_blurShader->render(4, 0);
	
	m_renderer->setRenderTargets(&m_bloomTargets[1], 1);
	bloomSource = m_bloomTargets[0]->getShaderResourceView();
	m_deviceContext->PSSetShaderResources(0, 1, &bloomSource);
	m_blurShader->setLuminanceThreshold(0.0f);
	m_blurShader->setTechnique(Y_TECHNIQUE);
	m_blurShader->render(4, 0);

	ID3D11UnorderedAccessView* temp[] = {0};
	m_deviceContext->CSSetUnorderedAccessViews(0, 1, temp, 0);
	ID3D11ShaderResourceView* temp2[] = {0, 0, 0, 0, 0, 0};
	m_deviceContext->CSSetShaderResources(0, 6, temp2);
}

void Scene::renderFinalCombine()
{
	m_renderer->setBackBufferRenderTarget();
	m_renderer->resetViewport();

	ID3D11ShaderResourceView* color = m_lightingTarget->getShaderResourceView();
	ID3D11ShaderResourceView* sslr = m_sslrTarget->getShaderResourceView();
	ID3D11ShaderResourceView* ssao = m_ssaoTargets[0]->getShaderResourceView();
	ID3D11ShaderResourceView* particle = m_particleTarget->getShaderResourceView();
	ID3D11ShaderResourceView* bloom = m_bloomTargets[1]->getShaderResourceView();
	ID3D11ShaderResourceView* srvs[] = {color, sslr, ssao, particle, bloom};
	m_deviceContext->PSSetShaderResources(0, 5, srvs);

	m_combineTonemapShader->render(4, 1);

	ID3D11ShaderResourceView* temp[] = {0, 0, 0, 0};
	m_deviceContext->PSSetShaderResources(0, 4, temp);
}