#version 450

in Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;

//uniform sampler2D _MainTexture;
uniform sampler2D _Albedo;
uniform sampler2D _ZAtoon;

// Light source
uniform vec3 _EyePos;
uniform vec3 _lightDirection = vec3(0.0, 1.0, 0.0);

struct Material{
	float Ka; //Ambient coefficient (0-1)
	float Kd; //Diffuse coefficient (0-1)
	float Ks; //Specular coefficient (0-1)
	float Shininess; //Affects size of specular highlights
};
uniform Material _Material;

struct Palette{
	vec3 highlight;
	vec3 shadow;
};
uniform Palette _Palette;

out vec4 FragColor;

vec3 toonLighting(vec3 normal, vec3 lightDirection)
{
	float diff = (dot(normal, lightDirection) + 1.0) * 0.5;

	vec3 lightColor = vec3(1.0) * diff;
	float toonColor = texture(_ZAtoon, vec2(diff)).r;

	//Mix with palette
	lightColor = mix(lightColor, _Palette.highlight, _Palette.shadow);

	return lightColor * toonColor;
}

void main()
{
	vec3 normal = normalize(vs_out.worldNormal);

	vec3 lightColor = toonLighting(normal, _lightDirection);
	vec3 objectColor = texture(_Albedo, vs_out.texcoord).rgb;

	//Shade with 0-1 normal
	//FragColor = vec4(vs_out.normal * 0.5 + 0.5, 1.0f);
	//FragColor = texture(_MainTexture, vs_out.texcoord);
	FragColor = vec4(objectColor * lightColor, 1.0);
}