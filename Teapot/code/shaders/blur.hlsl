Texture2D textureMap : register(t0);
SamplerState wrapSampler : register(s0);

cbuffer PSConstants
{
	float4 pixelSizeSigmaThreshold;
};

static const float2 positions[] = {float2(-1, -1), float2(-1, 1), float2(1, -1), float2(1, 1)}; 

struct PixelInput
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

float calcGaussianWeight(int sampleDist, float sigma)
{
	float weight = 0;
	if (sigma > 0)
	{
		float g = 1.0f / sqrt(2.0f * 3.14159 * sigma * sigma);  
		weight = (g * exp(-(sampleDist * sampleDist) / (2 * sigma * sigma)));
	}
	return weight;
}

static const float PI = 3.1415927f;
static const int TAPS = 10;

float4 brightPassGaussianBlur(float2 texCoord, float sigma, float2 direction, float luminanceThreshold, float2 pixelSize)
{
	float4 color = 0;
	float weightSum = 0;
	[unroll]
	for (float i = -TAPS; i <= TAPS; i += 2)
	{	
		float4 sampled = textureMap.SampleLevel(wrapSampler, texCoord + i * direction * pixelSize, 0);

		float weight = calcGaussianWeight(i, sigma);
		weightSum += weight;
		if (exp(sampled.w) > luminanceThreshold)
			color += sampled * weight;
	}
	return color * rcp(weightSum);
}

PixelInput vertexShader(uint vertexId : SV_VertexID)
{
	float2 pos = positions[vertexId];

	PixelInput output;
	output.position = float4(pos, 1.0f, 1.0f);
	output.texCoord = pos * float2(0.5f, -0.5f) + 0.5f;
	
	return output;
}

float4 pixelShaderX(float4 position : SV_Position) : SV_Target
{
	float2 pixelSize = pixelSizeSigmaThreshold.xy;
	float2 texCoord = floor(position.xy) * pixelSize;
	float sigma = pixelSizeSigmaThreshold.z;
	float threshold = pixelSizeSigmaThreshold.w;
	return brightPassGaussianBlur(texCoord, sigma, float2(1, 0), threshold, pixelSize);
}

float4 pixelShaderY(float4 position : SV_Position) : SV_Target
{
	float2 pixelSize = pixelSizeSigmaThreshold.xy;
	float2 texCoord = floor(position.xy) * pixelSize;
	float sigma = pixelSizeSigmaThreshold.z;
	float threshold = pixelSizeSigmaThreshold.w;
	return brightPassGaussianBlur(texCoord, sigma, float2(0, 1), threshold, pixelSize);
}