#version 450

in Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

uniform vec3 _PortalColor;

out vec4 FragColor;

void main()
{
	FragColor = vec4(_PortalColor, 1.0);
}