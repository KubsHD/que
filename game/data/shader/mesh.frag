// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#version 450

layout(set = 0,binding = 0) uniform SceneData {
	mat4 viewProj;
	mat4 view;
	mat4 proj;
	vec3 camPos;
};

layout(set = 1, binding = 0) uniform InstanceData {
	mat4 model;
	mat4 modelInvTrans;
};

layout(set = 1, binding = 1) uniform sampler2D tex_diffuse;
layout(set = 1, binding = 2) uniform sampler2D tex_normal;
layout(set = 1, binding = 3) uniform sampler2D tex_orm;

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in vec3 i_Pos;
layout(location = 3) in vec3 i_WorldPos;

layout(location = 0) out vec4 o_Color;

// https://learnopengl.com/PBR/Lighting

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}  


float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

void main() {

	float metallic = texture(tex_orm, i_TexCoord).z;
	float roughness = texture(tex_orm, i_TexCoord).y;
	float ao = 0.0f;
	vec3 lightPos = vec3(-3.0f, 4.0f, 0.0f);
	vec3 lightColor = vec3(150.0f);
	
	vec3 norm = i_Normal;
	//norm = normalize(norm * 2.0 - 1.0);   

	vec3 albedo = texture(tex_diffuse,i_TexCoord).xyz;

	vec3 viewDir = normalize(camPos - i_WorldPos);
	vec3 lightDir = normalize(lightPos); 
	vec3 h = normalize(viewDir + lightDir);

	float distance = length(lightPos);
	float attenuation = 1.0 / (distance * distance);
	vec3 radiance = lightColor * attenuation;

	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

	float NDF = DistributionGGX(norm, h, roughness);
	float G = GeometrySmith(norm, viewDir, lightDir, roughness);
	vec3 F  = fresnelSchlick(max(dot(h, viewDir), 0.0), F0);
	
	vec3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(norm, viewDir), 0.0) * max(dot(norm, lightDir), 0.0)  + 0.0001;
	vec3 specular     = numerator / denominator;  
	
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS;
  
	kD *= 1.0 - metallic;

    float NdotL = max(dot(norm, lightDir), 0.0);        
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.05) * albedo * ao;
    vec3 color = ambient + albedo;
	
    //color = color / (color + vec3(1.0));
    //color = pow(color, vec3(1.0/2.2));  

	o_Color = vec4(Lo, 1.0f);
}