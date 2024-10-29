#include "common.hlsli"

struct VSOutput {
    [[vk::location(0)]] float2 texCoord : TEXCOORD;
    [[vk::location(1)]] float3 normal : NORMAL;
    [[vk::location(2)]] float4 position : SV_POSITION;
    [[vk::location(3)]] float3 worldPos : WORLDPOS;
    [[vk::location(4)]] float3x3 TBN : TBN;
};

VSOutput vs_main(VertexInput input, uint VertexIndex: SV_VertexID) {
    VSOutput output;
    output.texCoord = input.texCoord;
    output.normal = input.normal;
    output.position = mul(Scene.viewProj, float4(input.position, 1.0f));
    output.worldPos = input.position;
    output.TBN = float3x3(input.tangent, input.bitangent, input.normal);
    return output;
}

struct ColorData {
	float3 color;
};

[[vk::push_constant]] ColorData PushConstants;

float4 ps_main(VSOutput input) : SV_TARGET {
    return tex_diffuse.Sample(tex_diffuse_sm, input.texCoord);
}
