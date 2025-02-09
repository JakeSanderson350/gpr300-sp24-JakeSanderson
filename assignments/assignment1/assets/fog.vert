#version 450

//Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;

uniform mat4 _TransformModel;

out vec2 vsTexcoord;
out vec3 vsWorldPos;

void main()
{
	vsTexcoord = inTexcoord;
	vsWorldPos = vec3(_TransformModel * vec4(inPosition, 1.0));

	gl_Position = vec4(inPosition.xy, 0.0, 1.0);
}