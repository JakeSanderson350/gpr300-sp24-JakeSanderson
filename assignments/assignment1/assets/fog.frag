#version 450

uniform sampler2D _MainTexture;
uniform sampler2D _DepthTexture;
uniform vec3 _CameraPos;

in vec2 vsTexcoord;
in vec3 vsWorldPos;

out vec4 FragColor;

const float fogMax = 15.0;
const float fogMin = 5.0;

float getFogFactor(float _distance)
{
	if (_distance >= fogMax)
		return 1;
	if (_distance <= fogMin)
		return 0;

	return 1 - (fogMax - _distance) / (fogMax - fogMin);
}

void main()
{
	float dist = distance(_CameraPos, vsWorldPos);
	float alpha = getFogFactor(dist);

	FragColor = mix(texture(_MainTexture, vsTexcoord), vec4(vec3(0.8), 1.0), alpha);
}