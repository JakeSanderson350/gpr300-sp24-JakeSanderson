#version 450

in Surface{
	vec3 worldPos;
	vec4 lightPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

uniform sampler2D _MainTexture;
uniform sampler2D _ShadowMap;

// Light source
uniform vec3 _CamPos;
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

uniform float _MinBias, _MaxBias;

out vec4 FragColor;

float shadowCalculation(vec4 _lightPos)
{
	vec3 projCoords = _lightPos.xyz / _lightPos.w;
	projCoords = (projCoords * 0.5) + 0.5;

	float lightDepth = texture(_ShadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;

	vec3 lightDir = normalize(_Light.lightPos - vs_out.worldPos);
	float bias = max(_MaxBias * (1.0 - dot(vs_out.worldNormal, lightDir)), _MinBias);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(_ShadowMap, 0);

	for (int x = -1; x <= 1; ++x)
	{
		for (int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(_ShadowMap, projCoords.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	//return currentDepth - bias > lightDepth ? 1.0 : 0.0;
	return shadow /= 9.0;
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
	vec3 specular = pow(ndoth, _Material.Shininess) * vec3(_Material.Ks);

	return diffuse + specular;
}

void main()
{
	vec3 bpLighting = blinnphong(vs_out.worldNormal, vs_out.worldPos);
	vec3 ambient = _Material.Ka * _Light.lightColor;
	float shadow = shadowCalculation(vs_out.lightPos);

//	lighting *= (1.0 - shadow);
//	lighting *= _Light.lightColor;
	vec3 objectColor = vs_out.worldNormal * 0.5 + 0.5;

	//vec3 lighting = (ambient + (1.0 - shadow)) * bpLighting;
	bpLighting *= (1.0 - shadow);
	bpLighting *= ambient;
	bpLighting *= _Light.lightColor;

	FragColor = vec4(objectColor * bpLighting, 1.0);
	//FragColor = texture(_ShadowMap, vs_out.texcoord);
}