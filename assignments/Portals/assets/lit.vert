#version 450

//Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;

out vec2 texcoord;

void main()
{
	texcoord = inTexcoord;

	//Transform vertex position to homogenous clip space
	gl_Position = vec4(inPosition, 1.0);
}