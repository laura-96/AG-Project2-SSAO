#version 330 core

in vec2 fragTexCoords;
uniform sampler2D texture;

out vec4 FragColor;

void main()
{
	if(fragTexCoords.x < 0.01 && fragTexCoords.y < 0.01)
		FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	else
	 FragColor = texture2D(texture, fragTexCoords);
}
