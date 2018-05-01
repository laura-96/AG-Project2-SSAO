#version 330 core

layout (location = 0) out vec3 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// Change it to uniforms
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

//Tile noise scale
uniform float screenWidth;
uniform float screenHeight;
uniform float tileSize;

vec2 noiseScale = vec2(screenWidth / tileSize, screenHeight / tileSize);

uniform mat4 projection;

void main()
{
	FragColor = vec3(1.0f);
	return;
	// Input from G-Buffer
	vec3 fragPos = texture(gPosition, TexCoords).xyz;
	vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
	vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

	// TBN matrix
	vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
	vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	float occlusion = 0.0;
	for(int i = 0; i < kernelSize; ++i)
	{
		vec3 samp = TBN * samples[i];
		samp = fragPos + samp * radius;

		vec4 offset = vec4(samp, 1.0);
		offset = projection * offset; // convert from view to screen-space
		offset.xyz /= offset.w; // divide prespective
		offset.xyz = offset.xyz * 0.5 + 0.5; // Range 0.0 to 1.0

		float sampleDepth = texture(gPosition, offset.xy).z;
		float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
		occlusion += (sampleDepth >= samp.z + bias ? 1.0 : 0.0) * rangeCheck;
	}

	occlusion = 1.0 - (occlusion / kernelSize);

	//FragColor = vec4(vec3(occlusion), 1.0f);
}
