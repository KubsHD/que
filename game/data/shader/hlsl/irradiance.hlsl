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
    float4 pos = mul(pc_sd.viewProj, float4(input.position, 1.0f));
    output.pos = pos.xyww;

    return output;
}

[[vk::binding(0,0)]]
[[vk::combinedImageSampler]]
TextureCube tex1;

[[vk::binding(0,0)]]
[[vk::combinedImageSampler]]
SamplerState tex1_sm;

static float PI = 3.14159265359;

float4 ps_main(VS_OUTPUT input) : SV_Target {
	float3 normal = normalize(input.local_position);
	float3 irradiance = float3(0,0,0);

	float3 up    = float3(0.0, 1.0, 0.0);
	float3 right = normalize(cross(up, normal));
	up         = normalize(cross(normal, right));

	float sampleDelta = 0.025;
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            irradiance += tex1.Sample(tex1_sm, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0f);
}