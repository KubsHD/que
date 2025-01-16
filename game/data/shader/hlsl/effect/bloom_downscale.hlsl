
#if COMPILE_VS

struct VSOutput {
    float4 position : SV_POSITION;
    float4 fragPosLightSpace : TEXCOORD2;
    float2 texCoord : TEXCOORD;
};

VSOutput vs_main(uint VertexIndex: SV_VertexID) {
        VSOutput o;

    return o;
}

#endif

#if COMPILE_PS

struct VSOutput {
    float4 position : SV_POSITION;
    float4 fragPosLightSpace : TEXCOORD2;
    float2 texCoord : TEXCOORD;
};

float4 ps_main(VSOutput input) : SV_TARGET {
    return 0;
}

#endif

#if COMPILE_CS 

RWTexture2D<float4> input : register(u0);
RWTexture2D<float4> Output : register(u1);

struct BloomPushConstants {
    float2 srcResolution;
};

[[vk::push_constant]]
BloomPushConstants pc;

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
[numthreads(16, 16, 1)]
void cs_main(uint3 GlobalInvocationID : SV_DispatchThreadID)
{
    float2 texCoordTarget = float2(GlobalInvocationID.x, GlobalInvocationID.y);

    // get source texture coordinates
    float2 texCoord = texCoordTarget * 2.0f;

    float2 srcTexelSize = 1.0 / pc.srcResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = input.Load(int3(texCoord.x - 2, texCoord.y + 2, 0)).rgb;
    float3 b = input.Load(int3(texCoord.x,     texCoord.y + 2, 0)).rgb;
    float3 c = input.Load(int3(texCoord.x + 2, texCoord.y + 2, 0)).rgb;

    float3 d = input.Load(int3(texCoord.x - 2, texCoord.y, 0)).rgb;
    float3 e = input.Load(int3(texCoord.x,     texCoord.y, 0)).rgb;
    float3 f = input.Load(int3(texCoord.x + 2, texCoord.y, 0)).rgb;

    float3 g = input.Load(int3(texCoord.x - 2, texCoord.y - 2, 0)).rgb;
    float3 h = input.Load(int3(texCoord.x,     texCoord.y - 2, 0)).rgb;
    float3 i = input.Load(int3(texCoord.x + 2, texCoord.y - 2, 0)).rgb;

    float3 j = input.Load(int3(texCoord.x - 1, texCoord.y + 1, 0)).rgb;
    float3 k = input.Load(int3(texCoord.x + 1, texCoord.y + 1, 0)).rgb;
    float3 l = input.Load(int3(texCoord.x - 1, texCoord.y - 1, 0)).rgb;
    float3 m = input.Load(int3(texCoord.x + 1, texCoord.y - 1, 0)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    float3 result = e * 0.125;
    result += (a + c + g + i) * 0.03125;
    result += (b + d + f + h) * 0.0625;
    result += (j + k + l + m) * 0.125;

    result = max(result, 0.0001f);

    Output[texCoordTarget].rgb = result;
}

#endif