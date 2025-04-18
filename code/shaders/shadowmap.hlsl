cbuffer Matrices
{
	matrix modelMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInput
{
    float4 position : POSITION;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXTURE0;
};

PixelInput vertexShader(VertexInput input)
{
    PixelInput output;
    input.position.w = 1.0f;
    output.position = mul(input.position, modelMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
	output.depthPosition = output.position;
	return output;
}