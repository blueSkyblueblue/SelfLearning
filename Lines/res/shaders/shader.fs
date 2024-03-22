#version 330 core

in vec3 v_Color;

out vec4 fragColor;

uniform vec3 u_Color;

void main()
{
	fragColor = vec4(v_Color, 0.8);
}