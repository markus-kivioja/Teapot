#define PIXELS_PER_TILE_X 16
#define PIXELS_PER_TILE_Y 16
#define PIXELS_PER_TILE (PIXELS_PER_TILE_X * PIXELS_PER_TILE_Y)
#define KERNEL_SIZE 16

static const float HEMISPHERE_RADIUS = 0.25f;

Texture2D<float4> depthLayer : register(t0);
Texture2D<float4> normalLayer : register(t1);
Texture2D<float4> randomRotations : register(t2);

RWTexture2D<float4> ssaoOutput : register(u0);

SamplerState linearSampler : register(s0);

groupshared float2 s_bufSize;
groupshared uint2 s_originsInTile[PIXELS_PER_TILE];
groupshared float s_originDepthsInTile[PIXELS_PER_TILE];
groupshared uint s_originCountInTile;

cbuffer Matrices
{
	matrix cameraProjection;
}

cbuffer KernelBuffer
{
	float4 kernelPoints[KERNEL_SIZE];
}

float3 screenToView(float2 screenPos, float viewDepth)
{
	float2 clipScreenPos = (float2(screenPos) + 0.5f) / s_bufSize * 
							float2(2.0f, -2.0f) + float2(-1.0f, 1.0f);
    float2 viewRay = float2(clipScreenPos.x / cameraProjection._11,
                            clipScreenPos.y / cameraProjection._22);
    
    return float3(viewRay * viewDepth, viewDepth);
}

[numthreads(PIXELS_PER_TILE_X, PIXELS_PER_TILE_Y, 1)]
void computeShader(uint2 groupId : SV_GroupID, uint2 threadIdInGroup : SV_GroupThreadID, uint2 dispatchThreadId : SV_DispatchThreadID)
{
	uint threadInGroup = threadIdInGroup.y * PIXELS_PER_TILE_X + threadIdInGroup.x;
	if (threadInGroup == 0)
	{
		float2 bufSize;
		depthLayer.GetDimensions(bufSize.x, bufSize.y);
		s_bufSize = bufSize;
		s_originCountInTile = 0;
	}

	GroupMemoryBarrierWithGroupSync();
	
	float depth = depthLayer[dispatchThreadId].x;
	
	if (depth > 0)
	{
		uint listIndex;
		InterlockedAdd(s_originCountInTile, 1, listIndex);
		s_originsInTile[listIndex] = dispatchThreadId;
		s_originDepthsInTile[listIndex] = depth;
	}

	GroupMemoryBarrierWithGroupSync();
	
	[branch]
	if (threadInGroup < s_originCountInTile)
	{
		uint2 pixel = s_originsInTile[threadInGroup];
		depth = s_originDepthsInTile[threadInGroup];
		
		float viewDepth = cameraProjection._43 * rcp(depth - cameraProjection._33);
		float3 origin = screenToView(pixel, viewDepth);

		float3 rotation = randomRotations.SampleLevel(linearSampler, float2(pixel) * 0.25f, 0).xyz; // 1/rotation texture width = 1/4 = 0.25
		
		float3 N = normalLayer[pixel].xyz;
		float3 T = normalize(rotation - N * dot(rotation, N));
		float3 B = cross(N, T);
		float3x3 tsToVs = float3x3(T, B, N);

		float occlusion = 0;
		for (int i = 0; i < KERNEL_SIZE; ++i)
		{
			float3 vsKernelPoint = mul(kernelPoints[i].xyz, tsToVs);
			vsKernelPoint = vsKernelPoint * HEMISPHERE_RADIUS + origin;

			float4 texCoord = float4(vsKernelPoint, 1.0f);
			texCoord = mul(texCoord, cameraProjection);
			texCoord.xy *= rcp(texCoord.w);
			texCoord.xy = texCoord.xy * float2(0.5f, -0.5f) + 0.5f;

			float depthSample = depthLayer.SampleLevel(linearSampler, texCoord.xy, 0).x;
			depthSample = cameraProjection._43 * rcp(depthSample - cameraProjection._33); // To view space
	  
			float rangeCheck = abs(origin.z - depthSample) < HEMISPHERE_RADIUS ? 1.0f : 0.0f;
			rangeCheck = any(texCoord.xy < 0) || any(texCoord.xy > 1) ? 0.0f : rangeCheck;
			occlusion += (depthSample <= vsKernelPoint.z ? 1.0f : 0.0f) * rangeCheck;
		}

		ssaoOutput[pixel] = 1.0f - (occlusion * rcp(KERNEL_SIZE));
	}
}