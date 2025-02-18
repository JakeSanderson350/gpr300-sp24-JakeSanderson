#version 450

in Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

uniform sampler2D _ShadowMap;

// Light source
uniform vec3 _CamPos;
uniform vec3 _LightPos;
uniform vec3 _LightColor = vec3(1.0, 0.0, 1.0);
uniform vec3 _AmbientColor = vec3(0.3, 0.4, 0.46);

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

struct AmbientLight{
	vec3 lightPos;
	vec3 lightDirection;
};

uniform Material _Material;
uniform Light _Light;

out vec4 FragColor;

float shadowCalculation(vec4 lightPos)
{
	return 0;
}

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
	vec3 specular = pow(ndoth, _Material.Shininess * 128.0) * vec3(_Material.Ks);

	return diffuse + specular;
}

void main()
{

}