#version 330 core

in vec4 vertexOCS;
in vec3 normalOCS;
in vec4 fColor;

out vec4 FragColor;

void main()
{
    FragColor = fColor;
}
