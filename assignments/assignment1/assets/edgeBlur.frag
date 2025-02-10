#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

const float OFFSET = 1.0 / 300.0;
const vec2 offsets[9] = vec2[](
	vec2(-OFFSET, OFFSET), // top-left
	vec2(0, OFFSET), // top-center
	vec2(OFFSET, OFFSET), // top-right

	vec2(-OFFSET, 0.0), // middle-left
	vec2(0.0, 0.0), // middle-center
	vec2(OFFSET, 0.0), // middle-right

	vec2(-OFFSET, -OFFSET), //bottom-left
	vec2(0.0, -OFFSET), // bottom-center
	vec2(OFFSET, -OFFSET) // bottom-right
);

uniform float _Strength;
const float kernel[9] = float[](
	1.0, 1.0, 1.0,
	1.0, -8.0, 1.0,
	1.0, 1.0, 1.0
);

void main()
{
	vec3 avg = vec3(0.0);

	for (int i = 0; i < 9; i++)
	{
		vec3 local = texture(_MainTexture, vsTexcoord + offsets[i]).rgb;
		avg += local * (kernel[i] / _Strength);
	}

	vec3 albedo = texture(_MainTexture, vsTexcoord).rgb * avg;

	FragColor = vec4(albedo, 1.0);
}