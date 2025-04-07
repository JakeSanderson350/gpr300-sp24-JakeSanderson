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
	float radius;
};

uniform sampler2D _Albedo;
uniform sampler2D _Position;
uniform sampler2D _Normal;

uniform Light _Light;
const int NUM_LIGHTS = 64;
uniform Light _Lights[NUM_LIGHTS];
uniform Material _Material;
uniform vec3 _CamPos;

out vec4 FragColor;

vec3 blinnphong(vec3 _normal, vec3 _fragPos, vec3 _lightPos)
{
	// normalize inputs
	vec3 viewDir = normalize(_CamPos - _fragPos);

	vec3 lightDir = normalize(_lightPos - _fragPos);
	vec3 halfwayDir = normalize(lightDir + viewDir);

	//dot products
	float ndot1 = max(dot(_normal, lightDir), 0.0);
	float ndoth = max(dot(_normal, halfwayDir), 0.0);

	//light components
	vec3 diffuse = ndot1 * vec3(_Material.Kd);
	vec3 specular = pow(ndoth, _Material.Shininess) * vec3(_Material.Ks);

	return diffuse + specular;
}

float attenuateExponential(float _dist, float _radius)
{
	float i = clamp(1.0 - pow(_dist / _radius, 4.0), 0.0, 1.0);
	return i * i;
}

float attenuateLinear(float _dist, float _radius)
{
	return clamp((_radius - _dist) / _radius, 0.0, 1.0);
}

vec3 calcPointLight(Light _light, vec3 _normal, vec3 _pos)
{
	vec3 diff = _light.lightPos - _pos;
	//Direction toward light position
	vec3 toLight = normalize(diff);
	//Blinn-phong calculations for diffuse + specular
	vec3 lightColor = (blinnphong(_normal, _pos, _light.lightPos)) * _light.lightColor;
	//Attenuation
	float d = length(diff); //Distance to lights
	lightColor *= attenuateExponential(d, _light.radius);
	return lightColor ;
}


void main()
{
	// get data from textures
	vec3 objectColor = texture(_Albedo, texcoord).xyz;
	vec3 position = texture(_Position, texcoord).xyz;
	vec3 normal = texture(_Normal, texcoord).xyz;

	// caclulate lighting
	vec3 totalLight = vec3(0);

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		totalLight += calcPointLight(_Lights[i], normal, position);
	}

	FragColor = vec4(objectColor * totalLight , 1.0);
}