#version 300 es

precision highp float;

uniform vec3 _lightColor;

out vec4 FragColor;

void main()
{
	FragColor = vec4(_lightColor, 1.0);
}