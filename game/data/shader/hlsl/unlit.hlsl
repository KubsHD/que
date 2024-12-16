#include "common.hlsli"
#include "shadows.hlsli"
#include "input.hlsli"

struct VSOutput {
    float4 position : SV_POSITION;
    float4 fragPosLightSpace : TEXCOORD2;
    float2 texCoord : TEXCOORD;
};

[[vk::push_constant]]
GPUDrawPushConstants pc;




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


float4 ps_main(VSOutput input) : SV_TARGET {
    float shadow = ShadowCalculation(input.fragPosLightSpace);
    return tex_diffuse.Sample(tex_diffuse_sm, input.texCoord) * shadow;
}

#endif