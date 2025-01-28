#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

const vec3 offset = vec3(0.009, 0.006, -0.006);
const vec2 direction = vec2(1.0);

void main()
{
	FragColor.r = texture(_MainTexture, vsTexcoord + (direction * vec2(offset.r))).r;
	FragColor.g = texture(_MainTexture, vsTexcoord + (direction * vec2(offset.g))).g;
	FragColor.b = texture(_MainTexture, vsTexcoord + (direction * vec2(offset.b))).b;
}