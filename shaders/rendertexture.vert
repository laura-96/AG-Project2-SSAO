#version 330 core

in vec3 vertex;
in vec2 vertTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = vertTexCoords;
    gl_Position = vec4(vertex, 1);
}
