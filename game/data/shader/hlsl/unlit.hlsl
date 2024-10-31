#include "common.hlsli"

struct VSOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

[[vk::push_constant]]
GPUDrawPushConstants pc;

VSOutput vs_main(VertexInput input, uint VertexIndex: SV_VertexID) {
    VSOutput output;
    output.texCoord = input.texCoord;
    output.position = mul(mul(float4(input.position, 1.0f), pc.model), Scene.viewProj);
    return output;
}

[[vk::binding(0,1)]]
[[vk::combinedImageSampler]]
Texture2D tex_diffuse : register(t1);

[[vk::binding(0,1)]]
[[vk::combinedImageSampler]]
SamplerState tex_diffuse_sm : register(s1);


float4 ps_main(VSOutput input) : SV_TARGET {
    return tex_diffuse.Sample(tex_diffuse_sm, input.texCoord);
}
