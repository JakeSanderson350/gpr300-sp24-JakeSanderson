#version 450

uniform sampler2D _MainTexture;
uniform sampler2D _DepthTexture;
uniform vec3 _CameraPos;

in vec2 vsTexcoord;
in vec3 vsWorldPos;

out vec4 FragColor;

uniform float _FogFar = 15.0;
uniform float _FogNear = 5.0;

float getFogFactor(float _distance)
{
	if (_distance >= _FogFar)
		return 1;
	if (_distance <= _FogNear)
		return 0;

	return 1 - (_FogFar - _distance) / (_FogFar - _FogNear);
}

void main()
{
	float dist = distance(_CameraPos, vsWorldPos);
	float alpha = getFogFactor(dist);

	FragColor = mix(texture(_MainTexture, vsTexcoord), vec4(vec3(0.8), 1.0), alpha);
}