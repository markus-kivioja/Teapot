#define PIXELS_PER_TILE_X 16
#define PIXELS_PER_TILE_Y 16
#define PIXELS_PER_TILE (PIXELS_PER_TILE_X * PIXELS_PER_TILE_Y)

static const float epsilon = 0.0004f;
static const float stepSizeInPixelsTimesTwo = 6.0f;

Texture2D<float4> depthLayer : register(t0);
Texture2D<float4> colorLayer : register(t1);
Texture2D<float4> gBufLayer2 : register(t2);

RWTexture2D<float4> sslrOutput : register(u0);

SamplerState linearSampler : register(s0);

groupshared float2 s_bufSize;
groupshared uint2 s_raysInTile[PIXELS_PER_TILE];
groupshared uint s_rayCountInTile;

cbuffer Matrices
{
	matrix cameraProjection;
}

float3 screenToView(float2 screenPos, float viewDepth, inout float2 csScreenPos)
{
	csScreenPos = (float2(screenPos) + 0.5f) / s_bufSize * 
							float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float2 viewRay = float2(csScreenPos.x / cameraProjection._11,
                            csScreenPos.y / cameraProjection._22);
    
    return float3(viewRay * viewDepth, viewDepth);
}

[numthreads(PIXELS_PER_TILE_X, PIXELS_PER_TILE_Y, 1)]
void computeShader(uint2 groupId : SV_GroupID, uint2 threadIdInGroup : SV_GroupThreadID, uint2 dispatchThreadId : SV_DispatchThreadID)
{
	uint threadInGroup = threadIdInGroup.y * PIXELS_PER_TILE_X + threadIdInGroup.x;
	if (threadInGroup == 0)
	{
		float2 bufSize;
		colorLayer.GetDimensions(bufSize.x, bufSize.y);
		s_bufSize = bufSize;
		s_rayCountInTile = 0;
	}
	
	GroupMemoryBarrierWithGroupSync();
	
	float shininess = gBufLayer2[dispatchThreadId].w;
	
	if (shininess >= 50.0f)
	{
		uint listIndex;
		InterlockedAdd(s_rayCountInTile, 1, listIndex);
		s_raysInTile[listIndex] = dispatchThreadId;
	}

	GroupMemoryBarrierWithGroupSync();
	
	[branch]
	if (threadInGroup < s_rayCountInTile)
	{
		uint2 pixel = s_raysInTile[threadInGroup];
		float3 normal = gBufLayer2[pixel].xyz;
		float depthSample = depthLayer[pixel].x;
		float viewDepth = cameraProjection._43 * rcp(depthSample - cameraProjection._33);
		float3 csScreenPos = depthSample;
		float3 vsPos = screenToView(pixel, viewDepth, csScreenPos.xy);
		float3 vsReflection = normalize(reflect(normalize(vsPos.xyz), normalize(normal)));
		
		// Transform the reflection vector from view space to clip space
		float3 vsPosReflect = vsPos.xyz + vsReflection;
		float3 csPosReflect = mul(float4(vsPosReflect, 1.0f), cameraProjection).xyz * rcp(vsPosReflect.z);
		float3 csReflection = csPosReflect - csScreenPos;
		
		// Scale the reflection vector so that xy-length is four pixels
		csReflection *= stepSizeInPixelsTimesTwo * rcp(s_bufSize.x * length(csReflection.xy));

		// From clip space to texture space
		float3 currentTexPos = csScreenPos + csReflection;
		currentTexPos.xy = currentTexPos.xy * float2(0.5f, -0.5f) + 0.5f;
		float3 lastTexPos = csScreenPos;
		lastTexPos.xy = lastTexPos.xy * float2(0.5f, -0.5f) + 0.5f;
		
		csReflection.xy *= float2(0.5f, -0.5f);
		
		uint maxSampleCount = s_bufSize.x * 0.25f;
		uint currentSampleCount = 0;
		
		float3 reflectColor;
		float currentSample;
		float lastDelta = 0;
		while (currentSampleCount < maxSampleCount)
		{
			currentSample = depthLayer[currentTexPos.xy * s_bufSize].x;
			float delta = currentSample - currentTexPos.z;
			if (0 < delta && delta < epsilon)
			{
				float lerpRatio = delta * rcp(delta + abs(lastDelta));		
				currentTexPos.xy = lerp(currentTexPos.xy, lastTexPos.xy, lerpRatio);
				float2 edgeFade = (0.5f - abs(0.5f - currentTexPos.xy)) * 2.8f;
				sslrOutput[pixel] = float4(colorLayer[currentTexPos.xy * s_bufSize].rgb, min(edgeFade.x, edgeFade.y));
				currentSampleCount = maxSampleCount;
			}
			else
			{
				currentSampleCount++;
				lastTexPos = currentTexPos;
				currentTexPos += csReflection;
				lastDelta = delta;
			}
			if (1 <= currentTexPos.z || currentTexPos.z <= 0 || any(currentTexPos.xy < 0) || any(currentTexPos.xy > 1))
				currentSampleCount = maxSampleCount;
		}
	}
}