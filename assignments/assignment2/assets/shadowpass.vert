#version 450

//Vertex attributes
layout(location = 0) in vec3 inPosition;


uniform mat4 _TransformModel;
uniform mat4 _LightViewproj;

void main()
{
	vec4 worldPos = _TransformModel * vec4(inPosition, 1.0);
	gl_Position = _LightViewproj * worldPos;
}