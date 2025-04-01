#version 450

layout(location = 3) out vec4 LightVolume;

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlights
};

struct Light{
	vec3 lightPos;
	vec3 lightColor;
};

in Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

uniform sampler2D _Albedo;
uniform sampler2D _Position;
uniform sampler2D _Normal;

uniform Light _Light;
uniform Material _Material;
uniform vec3 _CamPos;

vec3 blinnphong(vec3 _normal, vec3 _fragPos)
{
	// normalize inputs
	vec3 viewDir = normalize(_CamPos - _fragPos);
	vec3 lightDir = normalize(_Light.lightPos - _fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	//dot products
	float ndot1 = max(dot(_normal, lightDir), 0.0);
	float ndoth = max(dot(_normal, halfwayDir), 0.0);

	// light components
	vec3 diffuse = ndot1 * vec3(_Material.Kd);
	vec3 specular = pow(ndoth, _Material.Shininess) * vec3(_Material.Ks);

	return diffuse + specular;
}

void main()
{
	vec3 objectColor = vs_out.worldNormal.xyz * 0.5 + 0.5;
	LightVolume = vec4(1.0);
}