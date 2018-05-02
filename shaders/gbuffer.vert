#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;

out vec2 TexCoords;
out vec3 Normal;
out mat4 fProjection;

// Observer Coordinate System
out vec4 vertexOCS;
out vec3 fmatamb;
out vec3 fmatdiff;
out vec3 fmatspec;
out float fmatshin;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



void main()
{
	fmatamb = matamb;
	fmatdiff = matdiff;
	fmatspec = matspec;
	fmatshin = matshin;

    vertexOCS = view * model * vec4(aPos, 1.0);
    TexCoords = aTexCoords;
    fProjection = projection;
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    Normal = normalMatrix * aNormal;
    gl_Position = projection * vertexOCS;
}