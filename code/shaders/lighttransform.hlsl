struct PointLight
{
    float4 positionRadius;
    float4 color;
};

struct SpotLight
{
	float4 positionSin;
	float4 directionCos;
	float4 colorRcpSin;
};

cbuffer MatrixBuffer
{
	matrix viewMatrix;
};

StructuredBuffer<PointLight> wsPointLights : register(t0);
StructuredBuffer<SpotLight> wsSpotLights : register(t1);

RWStructuredBuffer<PointLight> vsPointLights : register(u0);
RWStructuredBuffer<SpotLight> vsSpotLights : register(u1);

[numthreads(64,1,1)]
void computeShader(uint threadId : SV_DispatchThreadID)
{
	uint pointLightCount, spotLightCount, temp;
	wsPointLights.GetDimensions(pointLightCount, temp);
	wsSpotLights.GetDimensions(spotLightCount, temp);
	
	if (threadId < pointLightCount)
	{
		float4 input = wsPointLights[threadId].positionRadius;
		input.w = 1;
		float4 output = mul(input, viewMatrix);
		output.w = wsPointLights[threadId].positionRadius.w;
	
		vsPointLights[threadId].positionRadius = output;
		vsPointLights[threadId].color = wsPointLights[threadId].color;
	}
	else if (threadId < pointLightCount + spotLightCount)
	{
		uint spotIdx = threadId - pointLightCount;
		
		float4 input = wsSpotLights[spotIdx].positionSin;
		input.w = 1;
		float4 output = mul(input, viewMatrix);
		output.w = wsSpotLights[spotIdx].positionSin.w;
		vsSpotLights[spotIdx].positionSin = output;
	
		float3 vsDir = normalize(mul(wsSpotLights[spotIdx].directionCos.xyz, (float3x3)viewMatrix));
		vsSpotLights[spotIdx].directionCos = float4(vsDir, wsSpotLights[spotIdx].directionCos.w);
	
		vsSpotLights[spotIdx].colorRcpSin = wsSpotLights[spotIdx].colorRcpSin;
	}
}