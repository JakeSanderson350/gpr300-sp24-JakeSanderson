#version 450

layout(location = 3) out vec4 LightVolume;

struct Light{
	vec3 lightPos;
	vec3 lightColor;
};

in Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

uniform Light _Light;

out vec4 FragColor;

void main()
{
	LightVolume = vec4(_Light.lightColor, 1.0);
}