#version 450

in vec2 texcoord;

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlights
	float Metallic;
	float Roughness;
};

struct Light{
	vec3 lightPos;
	vec3 lightColor;
};

uniform sampler2D _Albedo;
uniform sampler2D _Position;
uniform sampler2D _Normal;

const float PI = 3.14159265359;
uniform Light _Light;
const int NUM_LIGHTS = 64;
uniform Light _Lights[NUM_LIGHTS];
uniform Material _Material;
uniform vec3 _CamPos;

float VdotN = 0.0;
float LdotN = 0.0;
float NdotH = 0.0;

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

vec3 calcPointLight(Light _light, vec3 _normal, vec3 _pos){
	vec3 diff = _light.lightPos - _pos;
	//Direction toward light position
	vec3 toLight = normalize(diff);
	//TODO: Usual blinn-phong calculations for diffuse + specular
	vec3 lightColor = (blinnphong(_normal, _pos, _light.lightPos)) * _light.lightColor;
	//Attenuation
	float d = length(diff); //Distance to light
	//lightColor*=attenuate(d,light.radius); //See below for attenuation options
	return lightColor;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 geometrySchlick()
{
	return vec3(1.0);
}

float normalDistribution()
{
	float alpha2 = pow(_Material.Roughness, 4); // aplha = roughness^2

	float denom = PI * (pow(pow(NdotH, 2) * (alpha2 - 1.0) + 1.0, 2), 2);

	return alpha2 / denom;
}

// Describes specular
vec3 cookTorrence(vec3 _fresnel, float _roughness)
{
	vec3 numerator = normalDistribution() * _fresnel * geometrySchlick();
	float denominator = 4.0 * VdotN * LdotN;

	return numerator / denominator;
}

vec3 BRDF(vec3 _pos, vec3 _lightDir)
{
	vec3 viewDir = normalize(_CamPos - _pos);
	vec3 halfwayDir = normalize(_lightDir + viewDir);
	float HdotV = dot(halfwayDir, viewDir);
	vec3 lambert = vec3(1.0, 0.0, 0.0) / PI;
	vec3 Ks = fresnelSchlick(HdotV, lambert);
	//vec3 Ks = cookTorrence(fresnelSchlick(HdotV, lambert), _Material.Roughness);
	vec3 Kd = (vec3(1.0) - Ks) * (1.0 - _Material.Metallic);

	vec3 diffuse = vec3(Kd * lambert);
	vec3 specular = vec3(Ks * cookTorrence(fresnelSchlick(HdotV, lambert), _Material.Roughness));

	return diffuse + specular;
}

vec3 outgoingLight(vec3 _normal, vec3 _pos)
{
	vec3 emitted = vec3(0.0);
	vec3 radiance = vec3(0.0);
	float lightStep = 1.0 / NUM_LIGHTS;
	vec3 viewDir = normalize(_CamPos - _pos);

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		vec3 incomingLight = _Lights[i].lightColor;
		vec3 lightDir = normalize(_Lights[i].lightPos - _pos);
		vec3 halfwayDir = normalize(lightDir + viewDir);
		NdotH = max(dot(_normal, halfwayDir), 0.0);

		vec3 brdf = BRDF(_pos, lightDir);
		float NdotL = dot(_normal, lightDir);

		radiance += brdf * incomingLight * NdotL * lightStep;
	}

	return radiance + emitted;
}

void main()
{
	// get data from textures
	vec3 objectColor = texture(_Albedo, texcoord).xyz;
	vec3 position = texture(_Position, texcoord).xyz;
	vec3 normal = texture(_Normal, texcoord).xyz;

	// caclulate lighting
	vec3 totalLight = outgoingLight(normal, position);

	FragColor = vec4(/*objectColor **/ totalLight , 1.0);
}