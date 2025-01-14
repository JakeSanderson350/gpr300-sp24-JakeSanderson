#version 450

//Vertex attributes
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;


uniform mat4 transform_model;
uniform mat4 camera_viewproj;

out Surface{
	vec3 world_pos;
	vec3 world_normal;
	vec2 texcoord;
}vs_out;

void main()
{
	vs_out.world_pos = vec3(transform_model * vec4(in_position, 1.0));
	vs_out.world_normal = transpose(inverse(mat3(transform_model))) * in_normal;
	vs_out.texcoord = in_texcoord;

	//Transform vertex position to homogenous clip space
	gl_Position = camera_viewproj * transform_model * vec4(in_position, 1.0);
}