#version 450

//Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;


uniform mat4 _TransformModel;
uniform mat4 _CameraViewproj;
uniform vec3 _CameraPos;
uniform float _Time;
uniform float _Strength;

out Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

out vec3 toCamera;

float calculateSurface(float x, float y)
{
	float num = 0.0;
	num += (sin(x * 1.0 / _Strength + _Time * 1.0) + sin(x * 2.3 / _Strength + _Time * 1.5) + sin(x * 3.3 / _Strength + _Time * 0.4)) / 3.0;
	num += (sin(y * 0.2 / _Strength + _Time * 1.8) + sin(y * 1.8 / _Strength + _Time * 1.8) + sin(y * 2.8 / _Strength + _Time * 0.8)) / 3.0;

	return num;
}

void main()
{
	vec3 pos = inPosition;
	pos += calculateSurface(pos.x, pos.z);

	vs_out.worldPos = vec3(_TransformModel * vec4(pos, 1.0));
	vs_out.worldNormal = transpose(inverse(mat3(_TransformModel))) * inNormal;
	vs_out.texcoord = inTexcoord;

	toCamera = _CameraPos - vs_out.worldPos;

	//Transform vertex position to homogenous clip space
	gl_Position = _CameraViewproj * _TransformModel * vec4(vs_out.worldPos, 1.0);
}