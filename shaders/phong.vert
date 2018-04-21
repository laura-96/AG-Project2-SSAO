#version 330 core

in vec3 vertex;
in vec3 normal;
in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;

uniform mat4 projTransform;
uniform mat4 viewTransform;
uniform mat4 sceneTransform;

// Observer Coordinate System
out vec4 vertexOCS;
out vec3 normalOCS;
out vec3 fmatamb;
out vec3 fmatdiff;
out vec3 fmatspec;
out float fmatshin;

void main()
{
  fmatamb = matamb;
  fmatdiff = matdiff;
  fmatspec = matspec;
  fmatshin = matshin;
  mat3 normalMatrix = inverse(transpose(mat3 (viewTransform * sceneTransform)));
  normalOCS = normalize(vec3(normalMatrix * normal));
  vertexOCS = viewTransform * sceneTransform * vec4(vertex, 1);
  gl_Position = projTransform * vertexOCS;
}
