#version 450

uniform vec3 _PortalColor;

out vec4 FragColor;

void main()
{
	FragColor = vec4(_PortalColor, 1.0);
}