#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

const float gamma = 2.2;
uniform float _Exposure;

void main()
{
	vec3 hdrColor = texture(_MainTexture, vsTexcoord).rgb;

	// Exposure tone mapping
	vec3 mapped = vec3(1.0) - exp(-hdrColor * _Exposure);
	// Gamma correction
	mapped = pow(mapped, vec3(1.0 / gamma));

	FragColor = vec4(mapped, 1.0);
}