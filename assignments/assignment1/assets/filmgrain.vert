#version 450

//Vertex attributes
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexcoord;

out vec2 vsTexcoord;

void main()
{
	vsTexcoord = inTexcoord;

	gl_Position = vec4(inPosition.xy, 0.0, 1.0);
}