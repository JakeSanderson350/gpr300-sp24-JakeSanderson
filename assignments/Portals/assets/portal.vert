#version 450

//Vertex attributes
layout(location = 0) in vec3 inPosition;


uniform mat4 _TransformModel;
uniform mat4 _CameraViewproj;

void main()
{
	//Transform vertex position to homogenous clip space
	gl_Position = _CameraViewproj * _TransformModel * vec4(inPosition, 1.0);
}