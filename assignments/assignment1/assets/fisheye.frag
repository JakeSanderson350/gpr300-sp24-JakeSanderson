#version 450

uniform sampler2D _MainTexture;

in vec2 vsTexcoord;
out vec4 FragColor;

uniform float _Distortion;

void main()
{
	float uvTan = atan(vsTexcoord.x, vsTexcoord.y);
	float uvDistortion = sqrt(dot(vsTexcoord, vsTexcoord));
	uvDistortion = uvDistortion * (1.0 + _Distortion * uvDistortion * uvDistortion);

	FragColor = texture(_MainTexture, vec2(0.5) + vec2(sin(uvTan), cos(uvTan)) * uvDistortion);
}