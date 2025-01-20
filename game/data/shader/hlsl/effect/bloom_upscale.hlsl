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

Texture2D<float4> Input : register(t0);
SamplerState Input_sm : register(s0);

Texture2D<float4> Output_R : register(t1);
SamplerState Output_R_sm : register(s1);

RWTexture2D<float4> Output : register(u2);

struct BloomPushConstants {
    float2 srcResolution;
};

[[vk::push_constant]]
BloomPushConstants pc;


// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
[numthreads(8, 8, 1)]
void cs_main(uint3 GlobalInvocationID : SV_DispatchThreadID)
{
  

    float2 srcSize, dstSize;
    Input.GetDimensions(srcSize.x, srcSize.y);
    Output.GetDimensions(dstSize.x, dstSize.y);
    
    if (GlobalInvocationID.x >= dstSize.x || GlobalInvocationID.y >= dstSize.y)
        return;

    // Convert dispatch pixel coordinates to UV coordinates in source texture space
    float2 texCoord = GlobalInvocationID.xy / dstSize;
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions
    float x = 0.005f;
    float y = 0.005f;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = Input.SampleLevel(Input_sm, float2(texCoord.x - x, texCoord.y + y), 0).rgb;
    float3 b = Input.SampleLevel(Input_sm, float2(texCoord.x, texCoord.y + y), 0).rgb;
    float3 c = Input.SampleLevel(Input_sm, float2(texCoord.x + x, texCoord.y + y), 0).rgb;
    float3 d = Input.SampleLevel(Input_sm, float2(texCoord.x - x, texCoord.y), 0).rgb;
    float3 e = Input.SampleLevel(Input_sm, float2(texCoord.x, texCoord.y), 0).rgb;
    float3 f = Input.SampleLevel(Input_sm, float2(texCoord.x + x, texCoord.y), 0).rgb;
    float3 g = Input.SampleLevel(Input_sm, float2(texCoord.x - x, texCoord.y - y), 0).rgb;
    float3 h = Input.SampleLevel(Input_sm, float2(texCoord.x, texCoord.y - y), 0).rgb;
    float3 i = Input.SampleLevel(Input_sm, float2(texCoord.x + x, texCoord.y - y), 0).rgb;

    // Apply weighted distribution, using a 3x3 tent filter:
    // 1 | 1 2 1 |
    // -- * | 2 4 2 |
    // 16 | 1 2 1 |
    float3 upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;

    //upsample += Input.SampleLevel(Input_sm, texCoord, 0).rgb;

    Output[GlobalInvocationID.xy].rgb = upsample;
}

#endif


