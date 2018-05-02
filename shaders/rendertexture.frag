#version 330 core

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D texNoise;
uniform vec3 samples[64];

uniform mat4 projection;


int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

//Tile noise scale
uniform float screenWidth;
uniform float screenHeight;
uniform float tileSize;

vec2 noiseScale = vec2(screenWidth / tileSize, screenHeight / tileSize);

out vec4 FragColor;

float CalcOcclusion()
{
	vec3 pixelPos = texture(gPosition, TexCoords).xyz;
	vec3 pixelNormal = normalize(((texture(gNormal, TexCoords).rgb) + 1.0) * 0.5);
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
		offset = projection * offset; // convert from view to screen-space
		offset.xyz /= offset.w; // divide prespective
		offset.xyz = offset.xyz * 0.5 + 0.5; // Range 0.0 to 1.0

		float sampleDepth = texture(gPosition, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(pixelPos.z - sampleDepth));
		occlusion += (sampleDepth >= samp.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / kernelSize);

	return occlusion;
}

void main()
{
	
	//FragColor = vec4(CalcOcclusion(), 1.0);
	FragColor = texture(gAlbedoSpec, TexCoords) + vec4(vec3(CalcOcclusion()*0.2), 1.0);
}
