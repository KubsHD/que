#version 450

#extension GL_GOOGLE_include_directive : require
#include "pbr.glsl"
#include "common.glsl"

layout(set = 1, binding = 1) uniform sampler2D tex_diffuse;
layout(set = 1, binding = 2) uniform sampler2D tex_normal;
layout(set = 1, binding = 3) uniform sampler2D tex_orm;
layout(set = 1, binding = 4) uniform samplerCube tex_sky;
layout(set = 1, binding = 5) uniform sampler2D tex_emission;

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in vec3 i_Pos;
layout(location = 3) in vec3 i_WorldPos;
layout(location = 4) in mat3 i_TBN;

layout(location = 0) out vec4 o_Color;


void main() {
	// textures
	vec3 albedo = pow(texture(tex_diffuse, i_TexCoord).rgb, vec3(2.2));
	float ao = texture(tex_orm, i_TexCoord).x;
	float roughness = texture(tex_orm, i_TexCoord).y;
	float metallic = texture(tex_orm, i_TexCoord).z;

	ao = 1.0f;
	metallic = 0.9f;

	vec3 lightPos = vec3(-3.0f, 2.0f, 0.0f);
	vec3 lightColor = vec3(10.0f);
	
	/*
	vec3 norm = texture(tex_normal, i_TexCoord).xyz;
	norm = (norm * 2.0 - 1.0);   
	norm = normalize(i_TBN * norm);
	*/

	vec3 norm = i_Normal;

	vec3 viewDir = normalize(camPos - i_WorldPos);

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	// start light calc
	vec3 lightDir = normalize(lightPos); 
	vec3 h = normalize(viewDir + lightDir);

	float distance = length(lightPos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = lightColor;

	float NDF = DistributionGGX(norm, h, roughness);
	float G = GeometrySmith(norm, viewDir, lightDir, roughness);
    vec3 F    = fresnelSchlick(max(dot(h, viewDir), 0.0), F0);        

	vec3 kS = F;
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(norm, viewDir), 0.0) * max(dot(norm, lightDir), 0.0)  + 0.0001;
	vec3 specular     = numerator / denominator;  

	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

    float NdotL = max(dot(norm, lightDir), 0.0);        
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL + texture(tex_emission, i_TexCoord).xyz;

	// post directional light
	
	kS = fresnelSchlick(max(dot(norm, viewDir), 0.0), F0);
	kD = vec3(1.0f) - kS;
	kD *= 1.0 - metallic;
	vec3 irradiance = texture(tex_sky, norm).rgb;
	vec3 diffuse = irradiance * albedo;
	vec3 ambient = (kD * diffuse) * ao;

    vec3 color = ambient + Lo;
	
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  

	o_Color = vec4(color, 1.0f);
}