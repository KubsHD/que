#include "im3d_common.hlsli"

struct VSOutput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

#if COMPILE_VS

[[vk::push_constant]]
DrawData Data;

VSOutput vs_main(uint VertexIndex: SV_VertexID)
{
    const Im3dVertex v = Data.buffer[VertexIndex];

    VSOutput output;
    output.position = mul(v.Position, Data.viewProj);
    output.color = v.Color;
    return output;
}

#endif

#if COMPILE_PS

float4 ps_main(VSOutput input) : SV_TARGET
{
    return input.color;
}

#endif