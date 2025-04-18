Texture2D shadowMap : register(t0);
SamplerState clampSampler : register(s0);

cbuffer Matrices
{
	matrix modelMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
	matrix lightViewMatrix;
	matrix lightProjectionMatrix;
};

struct VertexInput
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 instancePos : TEXCOORD1;
	float3 instanceColor : TEXCOORD2;
};

struct PixelInput
{
    float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float4 lightViewPosition : TEXCOORD1;
	float3 color : TEXCOORD2;
};

PixelInput vertexShader(VertexInput input)
{
	float4x4 translation = float4x4(float4(1, 0, 0, 0),
								    float4(0, 1, 0, 0),
								    float4(0, 0, 1, 0),
								    float4(input.instancePos, 1));

	float4x4 worldMatrix = mul(modelMatrix, translation);
									
    input.position.w = 1.0f;
    PixelInput output;
    output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
	
	output.lightViewPosition = mul(input.position, worldMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightViewMatrix);
    output.lightViewPosition = mul(output.lightViewPosition, lightProjectionMatrix);
	
	output.tex = input.tex;
	
	output.color = input.instanceColor;
	
	return output;
}

float4 pixelShaderDrop(PixelInput input) : SV_Target
{
	float2 projectTexCoord;
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;
	float4 color = float4(0.7f, 0.7f, 0.7f, (1.0f-input.tex.y)*0.2f);
	if((0 <= projectTexCoord.x && projectTexCoord.x <= 1) && (0 <= projectTexCoord.y && projectTexCoord.y <= 1))
	{
		float lightViewDepth = input.lightViewPosition.z / input.lightViewPosition.w;
		float maxDepth = shadowMap.Sample(clampSampler, projectTexCoord).r;
		if(maxDepth > 0 && lightViewDepth - 0.001f >= maxDepth)
		{
			color *= 0.5f;
		}
	}
    return color;
}

float4 pixelShaderSplash(PixelInput input) : SV_Target
{
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float dist = length(input.tex - float2(0.5f, 0.5f));
	if (dist < 0.5f)
		color = float4(0.6f, 0.6f, 0.6f, 0.3f);
	float2 projectTexCoord;
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;
	if((0 <= projectTexCoord.x && projectTexCoord.x <= 1) && (0 <= projectTexCoord.y && projectTexCoord.y <= 1))
	{
		float lightViewDepth = input.lightViewPosition.z / input.lightViewPosition.w;
		float maxDepth = shadowMap.Sample(clampSampler, projectTexCoord).r;
		if(maxDepth > 0 && lightViewDepth - 0.001f >= maxDepth)
		{
			color *= 0.4f;
		}
	}
    return color;
}

float4 pixelShaderPointLight(PixelInput input) : SV_Target
{
	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float dist = length(input.tex - float2(0.5f, 0.5f));
	if (dist < 0.5f)
		color = float4(input.color, 1.0f) * 4.0f;
	return color;
}