#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in mat4 fProjection;

uniform sampler2D texNoise;
uniform vec3 samples[64];

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

//Tile noise scale
uniform float screenWidth;
uniform float screenHeight;
uniform float tileSize;

vec2 noiseScale = vec2(screenWidth / tileSize, screenHeight / tileSize);


void main()
{
	vec3 pixelPos = FragPos;
	vec3 pixelNormal = normalize(Normal);
	vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);
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
		offset = fProjection * offset; // convert from view to screen-space
		offset.xyz /= offset.w; // divide prespective
		offset.xyz = offset.xyz * 0.5 + 0.5; // Range 0.0 to 1.0

		float sampleDepth = pixelPos.z;//texture(gPosition, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pixelPos.z - sampleDepth));
		occlusion += (sampleDepth >= samp.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / kernelSize);
	
	gPosition = vec3(occlusion);
	gNormal = normalize(Normal);
	gAlbedoSpec.rgb = vec3(0.95);
}