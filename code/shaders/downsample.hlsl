static const float2 positions[] = {float2(-1, -1), float2(-1, 1), float2(1, -1), float2(1, 1)};

cbuffer PixelSize
{
	float4 pixelSize;
};

Texture2D textureMap : register(t0);
SamplerState samplerState : register(s0);

float calculateLuminance(float3 color)
{
    return max(dot(color, float3(0.265068f, 0.67023428f, 0.06409157f)), 0.0001f);
}

struct PixelInput
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

PixelInput vertexShader(uint vertexId : SV_VertexID)
{
	float2 pos = positions[vertexId];

	PixelInput output;
	output.position = float4(pos, 1.0f, 1.0f);
	output.texCoord = pos * float2(0.5f, -0.5f) + 0.5f;
	
	return output;
}

float4 pixelShader2x2(float4 position : SV_Position) : SV_Target
{
	float2 texCoord = int2(position.xy) * 2 * pixelSize.xy;
	float4 color = textureMap.SampleLevel(samplerState, texCoord, 0, int2(1, 1));
	color.w = log(calculateLuminance(color.xyz));
	return color;
}

float4 pixelShader4x4(float4 position : SV_Position) : SV_Target
{
	float2 texCoord = int2(position.xy) * 4 * pixelSize.xy;
	float4 color = textureMap.SampleLevel(samplerState, texCoord, 0, int2(1, 1));
	color += textureMap.SampleLevel(samplerState, texCoord, 0, int2(3, 1));
	color += textureMap.SampleLevel(samplerState, texCoord, 0, int2(1, 3));
	color += textureMap.SampleLevel(samplerState, texCoord, 0, int2(3, 3));
	color *= 0.25f;
	return color;
}