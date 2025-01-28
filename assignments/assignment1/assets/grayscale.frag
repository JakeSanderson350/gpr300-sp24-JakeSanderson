#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

void main()
{

	vec3 albedo = texture(_MainTexture, vsTexcoord).rgb;

	// naive
	//float average = (albedo.r + albedo.g + albedo.b) / 3.0;

	// realistic
	float average = ((0.2126 * albedo.r) + (0.7152 * albedo.r) + (0.0722 * albedo.g));

	FragColor = vec4(average, average, average, 1.0);
}