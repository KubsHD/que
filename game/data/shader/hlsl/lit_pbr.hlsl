#include "input.hlsli"
#include "common.hlsli"
#include "pbr.hlsli"
#include "shadows.hlsli"

#define SPOT_DEG_OFFSET 5

struct VSOutput {
    float4 position : SV_Position;
    float3 localPosition : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition: POSITION1;
    float3x3 TBN : TEXCOORD1;
    float4 fragPosLightSpace : TEXCOORD2;
};

#if COMPILE_VS

[[vk::push_constant]]
GPUDrawPushConstants pc;

VSOutput vs_main(VertexInput input, uint vertexIndex: SV_VertexID) {
    VSOutput vo;

	vo.texCoord = input.texCoord;
    vo.localPosition = input.position;
    vo.normal = input.normal;
    vo.worldPosition = mul(float4(input.position, 1.0f), pc.model);

    vo.fragPosLightSpace = mul(mul(mul(float4(input.position, 1.0f),pc.model), Scene.lightMtx), transpose(biasMat));

    vo.position = mul(float4(vo.worldPosition, 1.0f), Scene.viewProj);
    
    float3 T = normalize(float3((float3)mul(float4(input.tangent,   0.0), pc.model)));
    float3 B = normalize(float3((float3)mul(float4(input.bitangent,   0.0), pc.model)));
    float3 N = normalize(float3((float3)mul(float4(input.normal,   0.0), pc.model)));
    vo.TBN = float3x3(T, B, N);

    return vo;
}

#endif

#if COMPILE_PS

[[vk::binding(0, 1)]]
[[vk::combinedImageSampler]]
Texture2D tex_diffuse : register(t1);
[[vk::binding(0, 1)]]
[[vk::combinedImageSampler]]
SamplerState tex_diffuse_sm : register(s1);

[[vk::binding(1, 1)]]
[[vk::combinedImageSampler]]
Texture2D tex_normal : register(t2);
[[vk::binding(1, 1)]]
[[vk::combinedImageSampler]]
SamplerState tex_normal_sm : register(s2);

[[vk::binding(2, 1)]]
[[vk::combinedImageSampler]]
Texture2D tex_orm : register(t3);
[[vk::binding(2, 1)]]
[[vk::combinedImageSampler]]
SamplerState tex_orm_sm : register(s3);

[[vk::binding(3, 1)]]
[[vk::combinedImageSampler]]
Texture2D tex_emission : register(t4);
[[vk::binding(3, 1)]]
[[vk::combinedImageSampler]]
SamplerState tex_emission_sm : register(s4);

float3 calc_point_light(PointLight light, float3 normal, float3 fragPos, float3 viewDir, float3 albedo, float metallic, float roughness, float3 F0)
{
	float3 lightDir = normalize(-light.position - fragPos); 
	float3 h = normalize(viewDir + lightDir);

	float distance = length(light.position);
	float attenuation = 1.0 / (distance * distance);
	float3 radiance = light.color;

	float NDF = DistributionGGX(normal, h, roughness);
	float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    float3 F    = fresnelSchlick(max(dot(h, viewDir), 0.0), F0);        

	float3 kS = F;
	float3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0)  + 0.0001;
	float3 specular     = numerator / denominator;  

	float3 kD = float3(1.0, 1.0, 1.0) - kS;
	kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, lightDir), 0.0);        
    return (kD * albedo / PI + specular) * radiance * NdotL /* + tex_emission.Sample(tex_emission_sm, input.texCoord).xyz*/;
}

float3 calc_spot_light(SpotLight light, float3 normal, float3 fragPos, float3 viewDir, float3 albedo, float metallic, float roughness, float3 F0)
{

	float3 lightDir = normalize(light.position - fragPos); 


	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = cos(light.angle) - cos(light.angle + radians(SPOT_DEG_OFFSET));
	float intensity = clamp((theta - cos(light.angle + radians(SPOT_DEG_OFFSET))) / epsilon, 0.0, 1.0) * light.intensity;

	float distance = length(light.position - fragPos);

	if (distance < light.range)
	{
		float3 h = normalize(viewDir + lightDir);

		float attenuation = 1.0 / (distance * distance);
		float3 radiance = light.color * attenuation * intensity;

		float NDF = DistributionGGX(normal, h, roughness);
		float G = GeometrySmith(normal, viewDir, lightDir, roughness);
		float3 F    = fresnelSchlick(max(dot(h, viewDir), 0.0), F0);        

		float3 kS = F;
		float3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0)  + 0.0001;
		float3 specular     = numerator / denominator;  

		float3 kD = float3(1.0, 1.0, 1.0) - kS;
		kD *= 1.0 - metallic;

		float NdotL = max(dot(normal, lightDir), 0.0);        
		return (kD * albedo / PI + specular) * radiance * NdotL /* + tex_emission.Sample(tex_emission_sm, input.texCoord).xyz*/;
	}
	else
	{
		return float3(0.0f, 0.0f, 0.0f);
	}
}


float3 calc_dir_light(float3 normal, float3 fragPos, float3 viewDir, float3 albedo, float metallic, float roughness, float3 F0)
{
	float3 lightPos = float3( -0.2f, -1.0f, -0.3f);
	float3 lightColor = float3(23.47, 21.31, 20.79);

	float3 lightDir = normalize(-lightPos); 
	float3 h = normalize(viewDir + lightDir);

	float distance = length(lightPos);
	float attenuation = 1.0 / (distance * distance);
	float3 radiance = lightColor;

	float NDF = DistributionGGX(normal, h, roughness);
	float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    float3 F    = fresnelSchlick(max(dot(h, viewDir), 0.0), F0);        

	float3 kS = F;
	float3 numerator = NDF * G * F;
	float denominator = 4.0 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0)  + 0.0001;
	float3 specular     = numerator / denominator;  

	float3 kD = float3(1.0, 1.0, 1.0) - kS;
	kD *= 1.0 - metallic;

	kS = fresnelSchlick(max(dot(normal, viewDir), 0.0), F0);
	kD = float3(1.0f, 1.0f, 1.0f) - kS;
	kD *= 1.0 - metallic;

    float NdotL = max(dot(normal, lightDir), 0.0);        
    return (kD * albedo / PI + specular) * radiance * NdotL /* + tex_emission.Sample(tex_emission_sm, input.texCoord).xyz*/;
}

float4 ps_main(VSOutput input): SV_Target {

	// textures
	float3 albedo = pow(tex_diffuse.Sample(tex_diffuse_sm, input.texCoord).rgb, float3(2.2,2.2,2.2));
	float ao = tex_orm.Sample(tex_orm_sm, input.texCoord).x;
	float roughness = tex_orm.Sample(tex_orm_sm, input.texCoord).y;
	float metallic = tex_orm.Sample(tex_orm_sm, input.texCoord).z;

	ao = 1.0f;
	metallic = 0.9f;


	
	float3 norm = UnpackNormalMap(tex_normal.Sample(tex_normal_sm, input.texCoord)).xyz;
	norm = normalize(mul(norm, input.TBN));
	
	//float3 norm = i_Normal;

	float3 viewDir = normalize(Scene.camPos - input.worldPosition);

	float3 F0 = float3(0.04, 0.04, 0.04); 
	F0 = lerp(F0, albedo, metallic);

	// start light calc

	float3 Lo = float3(0.0, 0.0, 0.0);

	//for (int i = 0; i < Scene.pointLightCount; i++)
	//{
//		Lo += calc_point_light(Scene.pointLights[i], norm, input.worldPosition, viewDir, albedo, metallic, roughness, F0);
//	}

	for (int i = 0; i < Scene.spotLightCount; i++)
	{
		Lo += calc_spot_light(Scene.spotLights[i], norm, input.worldPosition, viewDir, albedo, metallic, roughness, F0);
	}

	// post directional light
	

	//float3 irradiance = tex_sky.Sample(tex_sky_sm, norm).rgb;
	float3 ambient = float3(0.001, 0.001, 0.001) * albedo * ao;
    float3 color = ambient + Lo;

    float shadow = ShadowCalculation(input.fragPosLightSpace);
	//color *= shadow;
	
    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0/2.2,1.0/2.2,1.0/2.2));  

	return float4(color, 1.0f);
}

#endif