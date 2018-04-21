#version 330 core

in vec4 vertexOCS;
in vec3 normalOCS;
in vec4 vertexColor;
in vec2 vertexTexCoords;

out vec4 FragColor;

uniform int tex1Loaded;
uniform int tex2Loaded;
uniform sampler2D tex1Texture;
uniform sampler2D tex2Texture;
uniform vec4 lightPos;
uniform vec3 lightCol;

const float threshold = 0.02;
const float mixVal = 0.5;
const float white = 0.1; // Values greater than this will be considered white

void main()
{
	 if(tex2Loaded == 1) // 2 textures
	 {
		vec4 tex1Col = texture2D(tex1Texture, vertexTexCoords);
		vec4 tex2Col = texture2D(tex2Texture, vertexTexCoords);
		
		if(tex2Col.r > threshold)
			if(tex2Col.r >= white)
				FragColor = mix(tex1Col, tex2Col, mixVal);
			else
				FragColor = mix(tex1Col, tex2Col, (1.0 - (white - tex2Col.r)) * mixVal);
		else
			FragColor = tex1Col;
	 } 
	 else if(tex1Loaded == 1) // 1 texture
	 {
		FragColor = texture2D(tex1Texture, vertexTexCoords);
	 } 
	 else // 0 textures
	 {
		FragColor = vertexColor;
	 }
}
