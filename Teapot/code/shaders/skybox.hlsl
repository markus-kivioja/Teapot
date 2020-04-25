TextureCube skyMap : register(t0);
SamplerState wrapSampler  : register(s0);

cbuffer Matrices
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct PixelInput
{
	float4 pos : SV_POSITION;
	float3 texCoord : TEXCOORD;
};

PixelInput vertexShader(float3 inPos : POSITION)
{
	PixelInput output;

	matrix wvpMatrix = mul(mul(worldMatrix, viewMatrix), projectionMatrix);
	output.pos = mul(float4(inPos, 1.0f), wvpMatrix);
	output.pos.z = 0.0f;

	output.texCoord = inPos;

	return output;
}

float4 pixelShader(PixelInput input) : SV_Target
{
	return float4(skyMap.Sample(wrapSampler, input.texCoord).rgb, 0.0f);
}
