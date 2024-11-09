#include "input.hlsli"
#include "common.hlsli"
#include "pbr.hlsli"


struct VSOutput {
    float4 position : SV_Position;
    float3 localPosition : POSITION;
    float2 texCoord : TEXCOORD;
    float3 normal : NORMAL;
    float3 worldPosition: POSITION1;
    float3x3 TBN : TEXCOORD1;
};

#if COMPILE_VS

[[vk::push_constant]]
GPUDrawPushConstants pc;

VSOutput vs_main(VertexInput input, uint vertexIndex: SV_VertexID) {
    VSOutput vo;
    vo.localPosition = input.position;
    vo.normal = input.normal;
    vo.worldPosition = mul(float4(input.position, 1.0f), pc.model);

    vo.position = mul(float4(vo.worldPosition, 1.0f), Scene.viewProj);
    
    float3 T = normalize(float3((float3)mul(float4(input.tangent,   0.0), pc.model)));
    float3 B = normalize(float3((float3)mul(float4(input.bitangent,   0.0), pc.model)));
    float3 N = normalize(float3((float3)mul(float4(input.normal,   0.0), pc.model)));
    vo.TBN = float3x3(T, B, N);

    return vo;
}

#endif

#if COMPILE_PS

float4 ps_main(): SV_Target {
    return float4(1.0, 0.0, 0.0, 1.0);
}

#endif