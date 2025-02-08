#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

uniform float _NoiseModifier;

void main()
{
	float noise = (fract(sin(dot(vsTexcoord, vec2(12.9898, 78.233) * 2.0)) * 43758.5453));
	vec4 tex = texture(_MainTexture, vsTexcoord);

	FragColor = tex - (noise * _NoiseModifier);
}