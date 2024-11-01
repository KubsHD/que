#include "common.hlsli"
#include "input.hlsli"


[[vk::push_constant]]
GPUDrawPushConstants pc;

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float3 pos_raw : POSITIONT;
};

VS_OUTPUT vs_main(VertexInput input, uint VertexIndex: SV_VertexID) {
    VS_OUTPUT output;

    output.pos_raw = input.position;

    float4x4 rotView = Scene.view;

    rotView._m03_m13_m23_m33 = 0.0f;
    float4 clipPos = mul(Scene.proj, mul(rotView, float4(output.pos_raw, 1.0)));
    float4 pos = mul(Scene.viewProj, mul(pc.model, float4(input.position, 1.0f)));

    output.pos = pos.xyww;

    return output;
}

[[vk::binding(0,1)]]
[[vk::combinedImageSampler]]
TextureCube tex1;

[[vk::binding(0,1)]]
[[vk::combinedImageSampler]]
SamplerState tex1_sm;

static float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float4 ps_main(VS_OUTPUT input) : SV_Target {
    float3 envColor = tex1.Sample(tex1_sm, input.pos).rgb;

	return float4(envColor, 1.0f);
}