#include "common.hlsli"
#include "input.hlsli"

struct VSOutput {
    float4 position : SV_POSITION;
    float4 fragPosLightSpace : POSITION1;
    float2 texCoord : TEXCOORD;
};

[[vk::push_constant]]
GPUDrawPushConstants pc;

VSOutput vs_main(VertexInput input, uint VertexIndex: SV_VertexID) {
    VSOutput output;
    output.texCoord = input.texCoord;
    output.fragPosLightSpace = mul(float4(input.position, 1.0f), Scene.lightMtx);
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
    float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth =  shadow_map.Sample(shadow_map_sm, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float shadow = currentDepth > closestDepth ? 0.7 : 1.0;

    return shadow;
}

float4 ps_main(VSOutput input) : SV_TARGET {
    float shadow = ShadowCalculation(input.fragPosLightSpace);
    return mul((1.0 - shadow), tex_diffuse.Sample(tex_diffuse_sm, input.texCoord));
}

#endif