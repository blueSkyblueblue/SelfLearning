#version 330 core

layout(location = 0) in vec3 a_Position;

uniform vec2 u_Transition;

void main()
{
    gl_Position = vec4(a_Position.xy + u_Transition, a_Position.z, 1.0);
}