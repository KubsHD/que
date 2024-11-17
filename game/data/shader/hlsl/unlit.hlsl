#include "common.hlsli"
#include "input.hlsli"

struct VSOutput {
    float4 position : SV_POSITION;
    float4 fragPosLightSpace : TEXCOORD2;
    float2 texCoord : TEXCOORD;
};

[[vk::push_constant]]
GPUDrawPushConstants pc;


// https://github.com/SaschaWillems/Vulkan/blob/master/shaders/hlsl/shadowmapping/scene.vert
static const float4x4 biasMat = float4x4(
	0.5, 0.0, 0.0, 0.5,
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0 );

VSOutput vs_main(VertexInput input, uint VertexIndex: SV_VertexID) {
    VSOutput output;
    output.texCoord = input.texCoord;
    output.fragPosLightSpace = mul(mul(mul(float4(input.position, 1.0f),pc.model), Scene.lightMtx), transpose(biasMat));
    output.position = mul(mul(float4(input.position, 1.0f), pc.model), Scene.viewProj);
    return output;
}

#if COMPILE_PS

[[vk::binding(0,1)]]
[[vk::combinedImageSampler]]
Texture2D tex_diffuse : register(t1);

[[vk::binding(0,1)]]
[[vk::combinedImageSampler]]
SamplerState tex_diffuse_sm : register(s1);

[[vk::binding(1,0)]]
[[vk::combinedImageSampler]]
Texture2D shadow_map : register(t2);

[[vk::binding(1,0)]]
[[vk::combinedImageSampler]]
SamplerState shadow_map_sm : register(s2);

float ShadowCalculation(float4 fragPosLightSpace)
{
	float4 shadowCoord = fragPosLightSpace / fragPosLightSpace.w;

    float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 )
	{
		float dist = shadow_map.Sample( shadow_map_sm, shadowCoord.xy).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z )
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

float4 ps_main(VSOutput input) : SV_TARGET {
    float shadow = ShadowCalculation(input.fragPosLightSpace);
    return tex_diffuse.Sample(tex_diffuse_sm, input.texCoord) * shadow;
}

#endif