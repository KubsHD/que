#include "input.hlsli"

struct VSOutput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

#if COMPILE_VS

struct GPUDrawPushConstants {
	row_major float4x4 model;
};


[[vk::push_constant]]
GPUDrawPushConstants pc;

VSOutput vs_main(VertexInput input, uint VertexIndex: SV_VertexID) {
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), pc.model);
    return output;
}

#endif

#if COMPILE_PS
float4 ps_main(VSOutput input) : SV_TARGET {
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

#endif