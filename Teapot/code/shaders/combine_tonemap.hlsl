float3 rgbToYxy(float3 color)
{				 
	float3x3 rgbToXYZ = {0.5141364f, 0.3238786f, 0.16036376f,
						 0.265068f, 0.67023428f, 0.06409157f,
					     0.0241188f, 0.1228178f, 0.84442666f};
						  
	float3 XYZ = mul(rgbToXYZ, color);
	float3 Yxy = XYZ.g;
    Yxy.gb = XYZ.rg * rcp(dot(1.0f, XYZ));
	
	return Yxy;
}

float3 YxyToRgb(float3 color)
{
	float3x3 XYZtoRGB = {2.5651f, -1.1665f, -0.3986f, 
						-1.0217f, 1.9777f, 0.0439f, 
						 0.0753f, -0.2543f, 1.1892f};

	float3 XYZ = color.r;
	XYZ.rb *= float2(color.g * rcp(color. b), (1.0f - color.g - color.b) * rcp(color.b));

	return mul(XYZtoRGB, XYZ);	
}

float computeKeyValue(float logarithmicAverageLuminance)
{
	return 1.03f - (2.0f / (2.0f + log10(logarithmicAverageLuminance + 1.0f)));
}

float scaleLuminance(float luminance, float logarithmicAverageLuminance, float keyValue)
{
	return luminance * keyValue / logarithmicAverageLuminance;
}

float reinhard(float scaledLuminance, float whiteLevel) 
{    
	return scaledLuminance * (1.0f + scaledLuminance / (whiteLevel * whiteLevel)) / (1.0f + scaledLuminance);
}

float3 calcExposedColor(float3 color, float avgLuminance, float threshold, /*float autoExposure, */float userExposure, out float exposure)
{    
	exposure -= threshold;
	return exp2(exposure) * color;
}

Texture2D colorTexture : register(t0);
Texture2D sslrTexture : register(t1);
Texture2D ssaoTexture : register(t2);
Texture2D particleTexture : register(t3);
Texture2D bloomTexture : register(t4);

SamplerState linearSampler : register(s0);

static const float2 corners[] = {float2(-1, -1), float2(-1, 1), float2(1, -1), float2(1, 1)}; 

struct PixelInput
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};

PixelInput vertexShader(uint vertexId : SV_VertexID)
{
	float2 corner = corners[vertexId];

	PixelInput output;
	output.position = float4(corner, 1.0f, 1.0f);
	output.texCoord = corner * float2(0.5f, -0.5f) + 0.5f;
	
	return output;
}

float4 pixelShader(PixelInput input) : SV_Target
{
	// float avgLuminance = loadUnnormalizedPixelR(TEXTURE_PARAMETER(postProcessInput5), int2(gp_frame, 0));
	// avgLuminance = avgLuminance * (1 - gp_staticAvgLuminanceBlend) + gp_staticAvgLuminance * gp_staticAvgLuminanceBlend;

   
	// // Calculate exposed color (Auto / user exposure)
	// float outExposure = 0;
	// color = calcExposedColor(color, avgLuminance, 0, gp_exposure, outExposure);

	// // Apply bloom
	// float3 bloomColor = Bloom(TEXTURE_PARAMETER(postProcessInput2), TexCoord, gp_bloomMagnitude);
	// bloomColor = calcExposedColor(bloomColor, avgLuminance, gp_bloomThreshold, gp_exposure, outExposure);

	// color += bloomColor;
	// //color = (1 - (1 - color) * (1 - bloomColor)); //screen blend
	
	// float3 colorYxy = convertRGBtoXYZtoYxy(color);

	// //Toe function to control the dark end of the image
	// float luminance = colorYxy.r;

	// //Compute automaticly suitable key value to be used in luminance normalization
	// float keyValue = computeKeyValue(avgLuminance);


	// //Normalize image luminance
	// luminance = scaleLuminance(luminance, avgLuminance, keyValue);

	// //Compress to LDR = Tonemap
	// luminance = reinhardWithWhitePoint(luminance, gp_whiteLevel);

	// //Convert back to RGB color space
	// colorYxy.r = luminance;

	// color = convertYxytoXYZtoRGB(colorYxy.rgb);
	
	float4 color = colorTexture.Sample(linearSampler, input.texCoord);
	float4 sslr = sslrTexture.Sample(linearSampler, input.texCoord);
	float4 ssao = ssaoTexture.Sample(linearSampler, input.texCoord);
	float4 particle = particleTexture.Sample(linearSampler, input.texCoord);
	float4 bloom = bloomTexture.Sample(linearSampler, input.texCoord);

	if (sslr.a > 0.0f)
		color = lerp(color, sslr, color.a * sslr.a);
	
	color *= ssao.r;
	
	return color + bloom;// + particle;
}	