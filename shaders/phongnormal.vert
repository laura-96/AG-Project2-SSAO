#version 330 core

in vec3 vertex;
in vec3 normal;
in vec4 color;
in vec2 texCoords;

uniform mat4 projTransform;
uniform mat4 viewTransform;
uniform mat4 sceneTransform;

// Observer Coordinate System
out vec4 vertexOCS;
out vec3 normalOCS;
out vec4 vertexColor;
out vec2 vertexTexCoords;

void main()
{
    vertexOCS = viewTransform * sceneTransform * vec4(vertex, 1);
    mat3 normalMatrix = inverse(transpose(mat3 (viewTransform * sceneTransform)));
    normalOCS = normalize(vec3(normalMatrix * normal));
    vertexColor = color;
    vertexTexCoords = texCoords;

    gl_Position = projTransform * vertexOCS;
}
