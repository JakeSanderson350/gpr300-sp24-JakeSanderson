#version 450

layout(location = 0) out vec4 FragAlbedo;
layout(location = 1) out vec4 FragPosition;
layout(location = 2) out vec4 FragNormal;

in Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

void main()
{
	vec3 objectColor = vs_out.worldNormal.xyz * 0.5 + 0.5;

	FragAlbedo = vec4(objectColor, 1.0);
	FragPosition = vec4(vs_out.worldPos, 1.0);
	FragNormal = vec4(vs_out.worldNormal, 1.0);
}