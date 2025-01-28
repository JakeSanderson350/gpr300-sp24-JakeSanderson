#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

void main()
{

	vec3 albedo = texture(_MainTexture, vsTexcoord).rgb;

	FragColor = vec4(albedo, 1.0);
}