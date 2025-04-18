cbuffer Matrices
{
	matrix modelMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInput
{
    float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelOutput
{
	float4 depth_normal : SV_Target0;
	float4 texCoord : SV_Target1;
};

PixelInput vertexShader(VertexInput input)
{
    PixelInput output;
    input.position.w = 1.0f;
    output.position = mul(input.position, modelMatrix);
    output.position = mul(output.position, viewMatrix);
	
	output.depthPosition = output.position;
	output.normal = mul(input.normal, (float3x3)modelMatrix);
    output.normal = normalize(output.normal);
	
    output.position = mul(output.position, projectionMatrix);
	
	output.tex = input.tex;
	
	return output;
}

PixelOutput pixelShader(PixelInput input)
{
	PixelOutput output;
	output.depth_normal = float4(input.depthPosition.z, input.normal.x, input.normal.y, input.normal.z);
	output.texCoord = float4(input.tex.x, input.tex.y, 0.0f, 0.0f);
	return output;
}
