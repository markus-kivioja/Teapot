#define PIXELS_PER_TILE_X 16
#define PIXELS_PER_TILE_Y 16
#define PIXELS_PER_TILE (PIXELS_PER_TILE_X * PIXELS_PER_TILE_Y)
#define MAX_POINT_LIGHT_COUNT 100
#define MAX_SPOT_LIGHT_COUNT 4

#define POINT_LIGHT 0
#define SPOT_LIGHT 1

static const float SHADOW_BIAS = 0.0004f;

struct PointLight
{
    float4 viewPosRad;
    float4 color;
};

struct SpotLight
{
	float4 viewPosSin;
	float4 directionCos;
	float4 colorRcpSin;
};

struct SurfacePoint
{
	float3 viewPos;
	float3 N;
	float specFactor;
	float shininess;
};

Texture2D<float4> gBufferDepth : register(t0);
Texture2D<float4> gBufferLayer1 : register(t1);
Texture2D<float4> gBufferLayer2 : register(t2);

Texture2DArray<float4> shadowMaps : register(t3);

StructuredBuffer<PointLight> pointLights : register(t4);
StructuredBuffer<SpotLight> spotLights : register(t5);

TextureCube skyMap : register(t6);

SamplerState wrapSampler  : register(s0);
SamplerComparisonState shadowSampler : register(s1);

RWTexture2D<float4> lightOutput : register(u0);

cbuffer Matrices
{
	matrix cameraProjection;
	matrix inverseView;
	matrix inverseViewLightVP[MAX_SPOT_LIGHT_COUNT];
}

float4 makePlaneFromCorners(float3 a, float3 b, float3 c)
{
	float4 result;
	result.xyz = -normalize(cross( b-a, c-a ));
	result.w = -dot(result.xyz, a);
	return result;
}

struct Frustum
{
	float4 m_plane[4];
};

groupshared uint2 s_lightCountsInTile;
groupshared uint s_pointLightsInTile[MAX_POINT_LIGHT_COUNT];
groupshared uint s_spotLightsInTile[MAX_SPOT_LIGHT_COUNT];
groupshared uint s_minMaxDepth[2];
groupshared float3 s_frustumCorners[8];
groupshared float2 s_bufferDimensions;
groupshared Frustum s_frustum;
groupshared uint2 s_totalLightCount;
groupshared float4 s_centerRadius;
groupshared bool s_bgTile;

#define intersectsPlane(planeIdx) \
	distance = dot( float4(sphere.x, sphere.y, sphere.z, 1.0f), s_frustum.m_plane[planeIdx]); \
	if (distance < -sphere.w) \
		return false;

bool intersectsFrustum(const float4 sphere)
{
	float radiusSum = s_centerRadius.w + sphere.w;
	float radiusSumSquared = radiusSum * radiusSum;
	float3 centerToSphere = s_centerRadius.xyz - sphere.xyz;
	float distanceSqrd = dot(centerToSphere, centerToSphere);
	
	if (distanceSqrd > radiusSumSquared)
		return false;
		
	float distance;
	intersectsPlane(0);
	intersectsPlane(1);
	intersectsPlane(2);
	intersectsPlane(3);
	
	return true;
}

bool intersectsFrustum(const SpotLight light)
{
	float3 expandedPos = light.viewPosSin.xyz - (s_centerRadius.w * light.colorRcpSin.w) * light.directionCos.xyz;
	float3 diff = s_centerRadius.xyz - expandedPos;
	float diffSqrd = dot(diff, diff);
	float e = dot(light.directionCos.xyz, diff);
	if (e > 0 && e * e >= diffSqrd * (light.directionCos.w * light.directionCos.w))
	{
		diff = s_centerRadius.xyz - light.viewPosSin.xyz;
		diffSqrd = dot(diff, diff);
		e = -dot(light.directionCos.xyz, diff);
		if (e > 0 && e * e >= diffSqrd * (light.viewPosSin.w * light.viewPosSin.w))
			return diffSqrd <= s_centerRadius.w * s_centerRadius.w;
		else
			return true;
	}
	return false;
}

float3 screenToView(float2 screenPos, float viewDepth)
{
	float2 clipScreenPos = (float2(screenPos) + 0.5f) / s_bufferDimensions * 
							float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float2 viewRay = float2(clipScreenPos.x / cameraProjection._11,
                            clipScreenPos.y / cameraProjection._22);
    
    return float3(viewRay * viewDepth, viewDepth);
}

void addBlinnPhongBRDF(PointLight light, SurfacePoint surfacePoint, inout float3 diffuse, inout float3 specular)
{
	float3 lightDir = light.viewPosRad.xyz - surfacePoint.viewPos;
	float lightDistSqrd = dot(lightDir, lightDir);
	float radiusSqrd = light.viewPosRad.w*light.viewPosRad.w;
	
	[branch]
	if (lightDistSqrd < radiusSqrd)
	{
		float attenuation = 1.0f - (lightDistSqrd * rcp(radiusSqrd));
		float3 L = normalize(lightDir);
		diffuse += light.color.xyz * saturate(dot(surfacePoint.N, L)) * attenuation;
		float3 E = normalize(-surfacePoint.viewPos);
		float3 H = normalize(E + L);
		specular += light.color.xyz * pow(saturate(dot(surfacePoint.N, H)), surfacePoint.shininess) *
					surfacePoint.specFactor * attenuation;
	}
}

void addBlinnPhongBRDF(SpotLight light, SurfacePoint surfacePoint, float shadow, inout float3 diffuse, inout float3 specular)
{
	float3 lightDir = surfacePoint.viewPos - light.viewPosSin.xyz;
	float3 normLightDir = normalize(lightDir);
	float cosLightAxisDir = dot(light.directionCos.xyz, normLightDir);
	[branch]
	if (light.directionCos.w < cosLightAxisDir)
	{
		float attenuation = (cosLightAxisDir - light.directionCos.w) * rcp(1.0f - light.directionCos.w) * 
							saturate(1.0f - dot(lightDir, lightDir) * 0.012f) * shadow;
		float3 L = -lightDir;
		diffuse += light.colorRcpSin.xyz * saturate(dot(surfacePoint.N, L)) * attenuation;
		float3 E = normalize(-surfacePoint.viewPos);
		float3 H = normalize(E + L);
		specular += light.colorRcpSin.xyz * pow(saturate(dot(surfacePoint.N, H)), surfacePoint.shininess) *
					surfacePoint.specFactor * attenuation;
	}
}

static const float m_2 = 0.5f;
static const float PI = 3.1415927f;

void addCookTorranceBRDF(PointLight light, SurfacePoint surfacePoint, inout float3 diffuse, inout float3 specular)
{
	float3 lightDir = light.viewPosRad.xyz - surfacePoint.viewPos;
	float lightDist = length(lightDir);
	if (lightDist <= light.viewPosRad.w)
	{
		float attenuation = 1.0f - (lightDist * rcp(light.viewPosRad.w));
		
		float3 L = normalize(lightDir);
		float3 E = normalize(-surfacePoint.viewPos);
		float3 H = normalize(E + L);
		float3 N = normalize(surfacePoint.N);
		
		float EdotH = dot(E, H);
		float NdotH = dot(N, H);
		float NdotL = dot(N, L);
		float NdotE = dot(N, E);
		
		float NdotHpow2 = NdotH * NdotH;
		float NdotHpow4 = NdotHpow2 * NdotHpow2;
		
		float g_min = min(NdotE, NdotL);
		float G = saturate(2 * NdotH * g_min / EdotH);
		float D = exp((NdotHpow2 - 1.0) * rcp(NdotHpow2 * m_2)) * rcp(PI * m_2 * NdotHpow4);
		float F = 0; // TODO: Fresnell term (use Schlick's approximation) R0 + (1.0 - R0) * pow(1.0 - EdotH, 5.0);
		
		float spec = (D*F*G) * rcp(4*(NdotE)*(NdotL));
		spec * light.color.xyz * pow(spec, surfacePoint.shininess) * surfacePoint.specFactor * attenuation;
	}
}

[numthreads(PIXELS_PER_TILE_X, PIXELS_PER_TILE_Y, 1)]
void computeShader(uint2 groupId : SV_GroupID, uint2 threadIdInGroup : SV_GroupThreadID, uint2 pixel : SV_DispatchThreadID)
{
	uint threadInGroup = threadIdInGroup.y * PIXELS_PER_TILE_X + threadIdInGroup.x;
	if (threadInGroup == 0)
	{
		s_lightCountsInTile[POINT_LIGHT] = 0;
		s_lightCountsInTile[SPOT_LIGHT] = 0;
		s_minMaxDepth[0] = 0x7F7FFFFF;
		s_minMaxDepth[1] = 0;
		
		float2 gBufSize;
		gBufferLayer1.GetDimensions(gBufSize.x, gBufSize.y);
		s_bufferDimensions = gBufSize;
		
		uint lightCount, temp;
		pointLights.GetDimensions(lightCount, temp);
		s_totalLightCount[POINT_LIGHT] = lightCount;
		
		spotLights.GetDimensions(lightCount, temp);
		s_totalLightCount[SPOT_LIGHT] = lightCount;
		
		s_bgTile = false;
	}
	float depthSample = gBufferDepth[pixel].x;
	float viewDepth = cameraProjection._43 * rcp(depthSample - cameraProjection._33);
	
	GroupMemoryBarrierWithGroupSync();
	
	if (0.0f < depthSample && depthSample < 1.0f)
	{
		InterlockedMin(s_minMaxDepth[0], asuint(viewDepth));
		InterlockedMax(s_minMaxDepth[1], asuint(viewDepth));
	}
	
	GroupMemoryBarrierWithGroupSync();

	if (threadInGroup < 8)
	{
		const uint a = (threadInGroup>>0)&1;
		const uint b = (threadInGroup>>1)&1;
		const uint c = (threadInGroup>>2)&1;
		const uint x = (groupId.x + (a^b)) * PIXELS_PER_TILE_X; 
		const uint y = (groupId.y + b) * PIXELS_PER_TILE_Y; 
		const float z = asfloat(s_minMaxDepth[c]);
		s_frustumCorners[threadInGroup] = screenToView(float2(x, y), z);
	}
	GroupMemoryBarrierWithGroupSync();
	
	if(threadInGroup < 4)
	{	
		s_frustum.m_plane[threadInGroup] = makePlaneFromCorners(s_frustumCorners[threadInGroup], s_frustumCorners[threadInGroup + 4], s_frustumCorners[(threadInGroup + 1) % 4]);
	}
	if(!threadInGroup)
	{
		float3 radius = 0.5f * (s_frustumCorners[6] - s_frustumCorners[0]);
		s_centerRadius.xyz = s_frustumCorners[0] + radius;
		s_centerRadius.w = length(radius);
		
		if (s_minMaxDepth[0] > s_minMaxDepth[1])
			s_bgTile = true;
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	uint totalLightCount = s_totalLightCount[POINT_LIGHT] + s_totalLightCount[SPOT_LIGHT];
	[loop]
	for (uint i = threadInGroup; i < totalLightCount; i += PIXELS_PER_TILE)
	{
		[branch]
		if (i < s_totalLightCount[POINT_LIGHT])
		{
			const float4 sphere = pointLights[i].viewPosRad;
			
			[branch]
			if (!s_bgTile && intersectsFrustum(sphere))
			{
				uint listIndex;
				InterlockedAdd(s_lightCountsInTile[POINT_LIGHT], 1, listIndex);
				s_pointLightsInTile[listIndex] = i;
			}
		}
		else
		{
			uint idx = i - s_totalLightCount[POINT_LIGHT];
			const SpotLight light = spotLights[idx];
			
			[branch]
			if (!s_bgTile && intersectsFrustum(light))
			{
				uint listIndex;
				InterlockedAdd(s_lightCountsInTile[SPOT_LIGHT], 1, listIndex);
				s_spotLightsInTile[listIndex] = idx;
			}
		}
    }
	
	float3 ambient = 0.2f;
	float3 diffuse = 0, specular = 0;

	float4 gBuf1Sample = gBufferLayer1[pixel];
	float4 gBuf2Sample = gBufferLayer2[pixel];
	
	SurfacePoint surfacePoint;
	surfacePoint.viewPos = screenToView(pixel, viewDepth);
	surfacePoint.N = gBuf2Sample.xyz;
	surfacePoint.specFactor = gBuf1Sample.a;
	surfacePoint.shininess = gBuf2Sample.a;
	
	GroupMemoryBarrierWithGroupSync();
	
	[loop]
	for (uint j = 0; j < s_lightCountsInTile[POINT_LIGHT]; ++j)
		addBlinnPhongBRDF(pointLights[s_pointLightsInTile[j]], surfacePoint, diffuse, specular);
	[loop]
	for (uint k = 0; k  < s_lightCountsInTile[SPOT_LIGHT]; ++k)
	{
		float4 lsPos = float4(surfacePoint.viewPos, 1.0f);
		uint idx = s_spotLightsInTile[k];
		lsPos = mul(lsPos, inverseViewLightVP[idx]);
		lsPos.xyz *= rcp(lsPos.w);
		lsPos.xy = float2(lsPos.x * 0.5f + 0.5f, -lsPos.y * 0.5f + 0.5f);
		float shadow = saturate(shadowMaps.SampleCmpLevelZero(shadowSampler, float3(lsPos.xy, idx), lsPos.z + SHADOW_BIAS, int2(-1, -1)));
		shadow += saturate(shadowMaps.SampleCmpLevelZero(shadowSampler, float3(lsPos.xy, idx), lsPos.z + SHADOW_BIAS, int2(-1, 1)));
		shadow += saturate(shadowMaps.SampleCmpLevelZero(shadowSampler, float3(lsPos.xy, idx), lsPos.z + SHADOW_BIAS, int2(1, 1)));
		shadow += saturate(shadowMaps.SampleCmpLevelZero(shadowSampler, float3(lsPos.xy, idx), lsPos.z + SHADOW_BIAS, int2(1, -1)));
		shadow *= 0.25f;
		[branch]
		if (shadow > 0)
			addBlinnPhongBRDF(spotLights[idx], surfacePoint, shadow, diffuse, specular);
	}
	
	float4 color;
	[branch]
	if (surfacePoint.shininess < 50.0f)
		color = float4(gBuf1Sample.rgb * (ambient + diffuse) + specular, 0.0f);
	else
	{
		float3 reflection = normalize(mul(reflect(normalize(surfacePoint.viewPos), surfacePoint.N), (float3x3)inverseView));
		float3 skyColor = skyMap.SampleLevel(wrapSampler, reflection, 0).rgb;
		float lerpRatio = (surfacePoint.shininess - 49.0f) / 101.0f;
		if (surfacePoint.shininess < 110.0f)
			color = float4(lerp(gBuf1Sample.rgb, skyColor, lerpRatio) * (ambient + diffuse) + specular, lerpRatio);
		else
			color = float4(skyColor * (ambient + diffuse) + specular, lerpRatio);
	}

	lightOutput[pixel] = color;
}
