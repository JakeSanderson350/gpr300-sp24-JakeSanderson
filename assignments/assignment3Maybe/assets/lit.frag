#version 450

in vec2 texcoord;

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

uniform sampler2D _Albedo;
uniform sampler2D _Position;
uniform sampler2D _Normal;

uniform Light _Light;
uniform Material _Material;
uniform vec3 _CamPos;

out vec4 FragColor;

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
	// get data from textures
	vec3 objectColor = texture(_Albedo, texcoord).xyz;
	vec3 position = texture(_Position, texcoord).xyz;
	vec3 normal = texture(_Normal, texcoord).xyz;

	// caclulate lighting
	vec3 bpLighting = blinnphong(normal, position);
	vec3 ambient = _Material.Ka * _Light.lightColor;
	bpLighting *= ambient;

	FragColor = vec4(objectColor , 1.0);
}