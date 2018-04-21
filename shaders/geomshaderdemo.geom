#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 256) out;

const float size = 10.0;
const float pi = 3.1415;

out vec4 fColor;

vec4 GetColor(float n)
{
	if(n > 2.0)
	{
		float i = n - 2.0;
		return vec4(1, 1, i, 1);

	}else if(n > 1.0)
	{
		float i = n - 1.0;
		return vec4(1, i, 0, 1);
	}
	else
	{
		return vec4(n, 0, 0, 1);
	}
	return vec4(0,0,0,0);
}

void main()
{
	float num_points = 32.0;
	float step = pi / (num_points / 2.0);
	

	vec4 C = gl_in[0].gl_Position;
	float tmpStep = (2.0*pi) - step;
	vec4 N = C + vec4(sin(tmpStep), cos(tmpStep), 0.0, 0.0) * size;
	vec4 B = N;

	float x = 0.0;
	float cb = 0.0;
	float cn = 0.0;
	for(x; x < 2.0*pi; x += step)
	{
		cb = cn;
		cn = (x / (2.0*pi)) * 3.0;

		fColor = GetColor(cb);
		gl_Position = C;
		EmitVertex();

		B = N;
		fColor = GetColor(cb);
		gl_Position = B;
		EmitVertex();
		
		N = C + vec4(sin(x), cos(x), 0.0, 0.0) * size;
		fColor = GetColor(cn);
		gl_Position = N;
		EmitVertex();

		EndPrimitive();
	}
}