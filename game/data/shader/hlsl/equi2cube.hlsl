#include "input.hlsli"

struct GPUEqui2CubeConstant {
	row_major float4x4 viewProj;
};


[[vk::push_constant]]
GPUEqui2CubeConstant pc_sd;

struct VS_OUTPUT {
    float4 pos : SV_POSITION;
    float3 local_position : POSITIONT;
};

VS_OUTPUT vs_main(VertexInput input, uint VertexIndex: SV_VertexID) {
    VS_OUTPUT output;

    output.local_position = input.position;
    float4 pos = mul(float4(input.position, 1.0f), pc_sd.viewProj);
    output.pos = pos.xyzw;

    return output;
}

[[vk::binding(0,0)]]
[[vk::combinedImageSampler]]
Texture2D tex1;

[[vk::binding(0,0)]]
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
    float2 uv = SampleSphericalMap(normalize(input.local_position)); 
	float3 color = tex1.Sample(tex1_sm, uv).xyz;
	return float4(color, 1.0f);
}