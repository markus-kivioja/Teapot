Texture2D textureMap : register(t0);
SamplerState samplerState : register(s0);

cbuffer PosSize
{
	float4 xyWidthHeight;
};

static const float2 corners[] = {float2(0, 0), float2(0, 1), float2(1, 0), float2(1, 1)}; 

struct PixelInput
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

PixelInput vertexShader(uint vertexId : SV_VertexID)
{
	float x = xyWidthHeight.x;
	float y = xyWidthHeight.y;
	float width = xyWidthHeight.z;
	float height = xyWidthHeight.w;

	float2 corner = corners[vertexId];
	
	PixelInput output;
	output.position = float4(x + corner.x * width, y + corner.y * height, 1.0f, 1.0f);
	output.texCoord = float2(corner.x, 1 - corner.y);
	
	return output;
}

float4 pixelShader(PixelInput input) : SV_Target
{
	return textureMap.Sample(samplerState, input.texCoord);
}