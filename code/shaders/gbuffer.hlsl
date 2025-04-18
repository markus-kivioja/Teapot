static const float PI = 3.1415927f;
static const float WAVE_FREQUENCY = 48.0f;
static const float WAVE_AMPLITUDE = 0.8f;
static const float WAVE_SPEED = 0.00018f;
static const float WAVE_DISTORTION_FACTOR = 0.015f;
static const float DRIBBLE_DISTORTION_FACTOR = 0.005f;
static const float WATER_SHININESS = 100.0f;
static const float DRIBBLE_RADIUS = 0.002f;

Texture2D colorMap : register(t0);
Texture2D normalMap : register(t1);
Texture2D displacementMap : register(t2);
Texture2D specularMap : register(t3);
Texture2D puddleMask : register(t4);

SamplerState linearSampler  : register(s0);

cbuffer VSconstants
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer PSconstants
{
	float4 shininess;
	matrix psViewMatrix;
};

cbuffer DropBuffer
{
	float4 dropPositions[100];
};

cbuffer PuddleBuffer
{
	float4 puddlePosSizes[5];
};

struct VertexInput
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float4 tangent : TANGENT;
};

struct PixelInput
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 T : TEXCOORD1;
	float3 B : TEXCOORD2;
	float3 N : TEXCOORD3;
	float3 wsT : TEXCOORD4;
	float3 wsB : TEXCOORD5;
	float3 wsN : TEXCOORD6;
	float3 wsPos : TEXCOORD7;
	float3 tsEyeNeg : TEXCOORD8;
};

struct PixelOutput
{
	float4 colorRGBspecularA : SV_Target0;
	float4 normalXYZshininessA : SV_Target1;
};

PixelInput vertexShader(VertexInput input)
{
	PixelInput output;
	input.position.w = 1.0f;
	
	output.position = mul(input.position, worldMatrix);
	
	output.wsPos = output.position.xyz;

	output.position = mul(output.position, viewMatrix);
	
	float3 vsPos = output.position.xyz;
	
	output.position = mul(output.position, projectionMatrix);

	float3 bitangent = input.tangent.w * cross(input.normal, input.tangent.xyz);
	
	output.wsT = normalize(mul(input.tangent.xyz, (float3x3)worldMatrix));
	output.wsB = normalize(mul(bitangent, (float3x3)worldMatrix));
	output.wsN = normalize(mul(input.normal, (float3x3)worldMatrix));
	
	matrix worldViewMatrix = mul(worldMatrix, viewMatrix);
	output.T = normalize(mul(input.tangent.xyz, (float3x3)worldViewMatrix));
	output.B = normalize(mul(bitangent, (float3x3)worldViewMatrix));
	output.N = normalize(mul(input.normal, (float3x3)worldViewMatrix));
	
	output.tsEyeNeg = float3(dot(vsPos, output.T), dot(vsPos, output.B), dot(vsPos, output.N));
	
	output.tex = input.tex;
	
	return output;
}

static const float DISPLACEMENT_SCALE = 0.025f;
static const int MAX_SAMPLES = 32;
static const int MIN_SAMPLES = 4;

void addParallaxOcclusion(float3 tsEye, inout float2 texCoords)
{
	float2 maxOffset = -tsEye.xy * rcp(tsEye.z) * DISPLACEMENT_SCALE;

	int maxSampleCount = (int)lerp(MAX_SAMPLES, MIN_SAMPLES, tsEye.z);
	
	float4 stepSize = rcp(maxSampleCount);
	stepSize.yzw = float3(stepSize.x * 2, stepSize.x * 3, stepSize.x * 4);
	float2 uvStep = stepSize.x * maxOffset;
	float4x2 uvStepSize = {uvStep, uvStep * 2, uvStep * 3, uvStep * 4};

	float2 lastOffset = float2(0.0f, 0.0f);
	
	float rayHeight = 1.0;	
	float lastHeightSample = 1.0f;
	float4 currentHeightSample = 1.0f;
	
	int samples = 0;
	while (samples < maxSampleCount)
	{
		// Take four steps in each loop
		currentHeightSample.x = displacementMap.SampleLevel(linearSampler, texCoords, 0).r;
		currentHeightSample.y = displacementMap.SampleLevel(linearSampler, texCoords + uvStepSize[0], 0).r;
		currentHeightSample.z = displacementMap.SampleLevel(linearSampler, texCoords + uvStepSize[1], 0).r;
		currentHeightSample.w = displacementMap.SampleLevel(linearSampler, texCoords + uvStepSize[2], 0).r;

		if (currentHeightSample.x > rayHeight)
		{
			float diff = currentHeightSample.x - rayHeight;
			float lastDiff = (rayHeight + stepSize.x) - lastHeightSample;
			float lerpRatio = diff * rcp(diff + lastDiff);		
			texCoords = lerp(texCoords, texCoords - uvStepSize[0], lerpRatio);
			return;
		}
		else if (currentHeightSample.y > rayHeight - stepSize.x)
		{
			float diff = currentHeightSample.y - (rayHeight - stepSize.x);
			float lastDiff = rayHeight - currentHeightSample.x;
			float lerpRatio = diff * rcp(diff + lastDiff);
			texCoords = lerp(texCoords + uvStepSize[0], texCoords, lerpRatio);
			return;
		}
		else if (currentHeightSample.z > rayHeight - stepSize.y)
		{
			float diff = currentHeightSample.z - (rayHeight - stepSize.y);
			float lastDiff = (rayHeight - stepSize.x) - currentHeightSample.y;
			float lerpRatio = diff * rcp(diff + lastDiff);
			texCoords = lerp(texCoords + uvStepSize[1], texCoords + uvStepSize[0], lerpRatio);
			return;
		}
		else if (currentHeightSample.w > rayHeight - stepSize.z)
		{
			float diff = currentHeightSample.w - (rayHeight - stepSize.z);
			float lastDiff = (rayHeight - stepSize.y) - currentHeightSample.z;
			float lerpRatio = diff * rcp(diff + lastDiff);
			texCoords = lerp(texCoords + uvStepSize[2], texCoords + uvStepSize[1], lerpRatio);
			return;
		}
		else
		{
			samples += 4;
			rayHeight -= stepSize.w;
			texCoords += uvStepSize[3];
			lastHeightSample = currentHeightSample.w;
		}
	}
}

PixelOutput pixelShaderRing(PixelInput input)
{
	float3 tsEye = normalize(input.tsEyeNeg);
	addParallaxOcclusion(tsEye, input.tex);
	bool willClip = any(input.tex.xy < 0) || any(input.tex.xy > 4);
	clip(willClip ? -1 : 1);

	float3 normal;
	
	float shine = shininess.x;
	uint puddleIdx = 0;
	[loop]
	for (int i = 1; i <= puddlePosSizes[0].x; ++i)
		if (all(puddlePosSizes[i].xy < input.wsPos.xz) && all(input.wsPos.xz < puddlePosSizes[i].xy + puddlePosSizes[i].zw))
		{
			puddleIdx = i;
			break;
		}
	[branch]
	if (puddleIdx)
	{
		float3 waveNormalWs = float3(0.0f, 1.0f, 0.0f);
		float2 distortionWs = 0;
		[loop]
		for (int i = 1; i <= dropPositions[0].x; ++i)
		{
			float2 diff = input.wsPos.xz - dropPositions[i].xz;
			float dist = length(diff);
			diff = normalize(diff);
			float lifeTime = dropPositions[i].w;
			[branch]
			if (dist < lifeTime * WAVE_SPEED)
			{
				float displacement = cos(dist * PI * WAVE_FREQUENCY - (lifeTime * WAVE_SPEED * PI * WAVE_FREQUENCY)) *
									 WAVE_AMPLITUDE * (1000.0f - lifeTime) * 0.001f;
				waveNormalWs += float3(diff.x * displacement, 1.0f - displacement, diff.y * displacement);
				distortionWs += -diff * displacement * WAVE_DISTORTION_FACTOR;
			}
		}
		float2 puddleMaskUV = float2((input.wsPos.x - puddlePosSizes[puddleIdx].x) / puddlePosSizes[puddleIdx].z,
									 (input.wsPos.z - puddlePosSizes[puddleIdx].y) / puddlePosSizes[puddleIdx].w);
		float lerpRatio = puddleMask.SampleLevel(linearSampler, puddleMaskUV, 0).r;
		
		float2 distortionTs = float2(dot(distortionWs, input.wsT.xz), dot(distortionWs, input.wsB.xz));
		input.tex += distortionTs * lerpRatio;
		
		normal = normalMap.SampleLevel(linearSampler, input.tex, 0).rgb * 2.0f - 1.0f;
		float3 waveNormalTs = float3(dot(waveNormalWs, input.wsT), dot(waveNormalWs, input.wsB), dot(waveNormalWs, input.wsN));
		normal = lerp(normal, waveNormalTs, lerpRatio);
		
		shine = lerp(shine, WATER_SHININESS, lerpRatio);
	}
	else
		normal = normalMap.SampleLevel(linearSampler, input.tex, 0).rgb * 2.0f - 1.0f;

	float3 T = normalize(input.T);
	float3 B = normalize(input.B);
	float3 N = normalize(input.N);
	float3x3 tsToVs = float3x3(T, B, N);
	normal = normalize(mul(normal, tsToVs));
	
	PixelOutput output;
	float4 color = colorMap.Sample(linearSampler, input.tex);
	float specular = specularMap.Sample(linearSampler, input.tex).r;
	output.colorRGBspecularA = float4(color.rgb, specular);
	output.normalXYZshininessA = float4(normal, shine);

    return output;
}

PixelOutput pixelShaderDribble(PixelInput input)
{
	float3 tsEye = normalize(input.tsEyeNeg);
	float height = (1.0f - displacementMap.Sample(linearSampler, input.tex)).r * 0.04f - 0.02f;
	input.tex += tsEye.xy * height;

	float3 dropNormalWs = 0;
	float3 distortionWs = 0;
	float shine = shininess.x;
	[loop]
	for (int i = 1; i <= dropPositions[0].x; ++i)
	{
		float3 diffWs = input.wsPos - dropPositions[i].xyz;
		float distSqrd = dot(diffWs, diffWs);
		if (distSqrd <= DRIBBLE_RADIUS)
		{
			float displacement = distSqrd * rcp(DRIBBLE_RADIUS);
			diffWs = normalize(diffWs);
			dropNormalWs += diffWs * displacement + input.wsN * (1 - displacement);
			distortionWs += -diffWs * displacement * DRIBBLE_DISTORTION_FACTOR;
		}
	}
	float3 normal;
	if (any(dropNormalWs))
	{
		normal = normalize(mul(dropNormalWs, (float3x3)psViewMatrix));
		input.tex += float2(dot(distortionWs, input.wsT), dot(distortionWs, input.wsB));
		shine = WATER_SHININESS;
	}
	else
	{
		float3 T = normalize(input.T);
		float3 B = normalize(input.B);
		float3 N = normalize(input.N);
		float3x3 tsToVs = float3x3(T, B, N);
		normal = normalMap.Sample(linearSampler, input.tex).rgb * 2.0f - 1.0f;
		normal = normalize(mul(normal, tsToVs));
	}
	
	PixelOutput output;
	float4 color = colorMap.Sample(linearSampler, input.tex);
	float specular = specularMap.Sample(linearSampler, input.tex).r;
	output.colorRGBspecularA = float4(color.rgb, specular);	
	output.normalXYZshininessA = float4(normal, shine);

    return output;
}