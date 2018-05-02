#version 330 core

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D texNoise;
uniform vec3 samples[64];

uniform mat4 projection;

uniform int useSSAO;
uniform int drawSSAO;

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

//Tile noise scale
uniform float screenWidth;
uniform float screenHeight;
uniform float tileSize;

vec2 noiseScale = vec2(screenWidth / tileSize, screenHeight / tileSize);
uniform float ssaoIntensity;

out vec4 FragColor;

float CalcOcclusion(vec2 texCoord)
{
	vec3 pixelPos = texture(gPosition, texCoord).xyz;
	vec3 pixelNormal = normalize(((texture(gNormal, texCoord).rgb) + 1.0) * 0.5);
	vec3 randomVec = normalize(texture(texNoise, texCoord * noiseScale).xyz);
	// TBN matrix
	vec3 tangent = normalize(randomVec - pixelNormal * dot(randomVec, pixelNormal));
	vec3 bitangent = cross(pixelNormal, tangent);
	mat3 TBN = mat3(tangent, bitangent, pixelNormal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		vec3 samp = TBN * samples[i];
		samp = pixelPos + samp * radius;

		vec4 offset = vec4(samp, 1.0);
		offset = projection * offset; // convert from view to screen-space
		offset.xyz /= offset.w; // divide prespective
		offset.xyz = offset.xyz * 0.5 + 0.5; // Range 0.0 to 1.0

		float sampleDepth = texture(gPosition, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pixelPos.z - sampleDepth));
		occlusion += (sampleDepth >= samp.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1 - (occlusion / kernelSize);

	return occlusion;
}

void main()
{
	vec4 pixel = texture(gAlbedoSpec, TexCoords);

	if(useSSAO == 1 || drawSSAO == 1)
	{
		float occlusion = 0.0;
		vec2 textelSize = 1.0 / vec2(screenWidth, screenHeight);
		for(int x = -2; x < 2; ++x)
		{
			for(int y = -2; y < 2; ++y)
			{
				vec2 offset = vec2(float(x), float(y)) * textelSize;
				occlusion += CalcOcclusion(TexCoords + offset); 
			}
		}
		occlusion = occlusion / 16.0;

		if(drawSSAO == 1)
		{
			FragColor = vec4(vec3(occlusion), 1.0);
			return;
		}

		pixel += vec4(vec3(1.0)*occlusion * ssaoIntensity, 1.0);
	}
	else
		pixel += vec4(vec3(1.0)*ssaoIntensity, 1.0);
	
	FragColor = pixel;
}
