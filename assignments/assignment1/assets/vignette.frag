#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

const float falloff = 0.5;
const float amount = 0.7;

void main()
{
	vec4 color = texture(_MainTexture, vsTexcoord);

	float dist = distance(vsTexcoord, vec2(0.5));
	color.rgb *= smoothstep(0.8, falloff * 0.799, dist * (amount + falloff));

	FragColor = color; 
}