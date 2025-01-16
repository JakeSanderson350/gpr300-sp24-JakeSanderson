#version 450

//Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;


uniform mat4 _TransformModel;
uniform mat4 _CameraViewproj;

out Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

void main()
{
	vs_out.worldPos = vec3(_TransformModel * vec4(inPosition, 1.0));
	vs_out.worldNormal = transpose(inverse(mat3(_TransformModel))) * inNormal;
	vs_out.texcoord = inTexcoord;

	//Transform vertex position to homogenous clip space
	gl_Position = _CameraViewproj * _TransformModel * vec4(inPosition, 1.0);
}