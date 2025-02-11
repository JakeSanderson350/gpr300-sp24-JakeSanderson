#version 450

in Surface{
	vec3 worldPos;
	vec3 worldNormal;
	vec2 texcoord;
}vs_out;
in vec3 toCamera;

uniform sampler2D _MainTexture;
uniform vec3 _WaterColor;
uniform vec3 _ReflectColor;
uniform float _Tiling;
uniform float _Time;
uniform vec2 _Direction;
uniform float _B1, _B2;

out vec4 FragColor;

const vec3 reflectColor = vec3 (1.0, 0.0, 0.0);

void main()
{
	//float tilingFactor = (sin(_Time) * 0.5) * _Tiling;
	float fresnelFactor = dot(normalize(toCamera), vec3(0.0, 1.0, 0.0));
	
	//vec2 dir = normalize(vec2(1.0));
	vec2 uv = (vs_out.texcoord * _Tiling) + (normalize(_Direction) * _Time) / 10.0;

	// distort water
	uv.y += 0.01 * (sin(uv.x * 3.5 + _Time * 0.35) + sin(uv.x * 4.8 + _Time * 1.05) + sin(uv.x * 7.3 + _Time * 0.45)) / 3.0;
    uv.x += 0.12 * (sin(uv.y * 4.0 + _Time * 0.5) + sin(uv.y * 6.8 + _Time * 0.75) + sin(uv.y * 11.3 + _Time * 0.2)) / 3.0;
    uv.y += 0.12 * (sin(uv.x * 4.2 + _Time * 0.64) + sin(uv.x * 6.3 + _Time * 1.65) + sin(uv.x * 8.2 + _Time * 0.45)) / 3.0;

	vec3 waterTexture = texture(_MainTexture, uv).xyz;
	
	vec4 smp1 = texture(_MainTexture, uv);
	vec4 smp2 = texture(_MainTexture, uv + vec2(0.2));

	//vec3 waterColor = mix(waterTexture * _ReflectColor, _WaterColor, fresnelFactor);
	vec3 color = _WaterColor + vec3(smp1.r * _B1 - smp2.r * _B2);
	FragColor = vec4(color, 1.0);
}