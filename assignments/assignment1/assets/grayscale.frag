#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

void main()
{

	vec3 albedo = texture(_MainTexture, vsTexcoord).rgb;

	float average = ((0.2126 * albedo.r) + (0.7152 * albedo.b) + (0.0722 * albedo.g));

	FragColor = vec4(average, average, average, 1.0);
}