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

float3 tentFilter(RWTexture2D<float4> tex, float2 texCoord)
{

    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = 0.003f;
    float y = 0.003f;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = tex.Load(int3(int2(texCoord.x - x, texCoord.y + y), 0));
    float3 b = tex.Load(int3(int2(texCoord.x,     texCoord.y + y), 0));
    float3 c = tex.Load(int3(int2(texCoord.x + x, texCoord.y + y), 0));

    float3 d = tex.Load(int3(int2(texCoord.x - x, texCoord.y), 0));
    float3 e = tex.Load(int3(int2(texCoord.x,     texCoord.y), 0));
    float3 f = tex.Load(int3(int2(texCoord.x + x, texCoord.y), 0));

    float3 g = tex.Load(int3(int2(texCoord.x - x, texCoord.y - y), 0));
    float3 h = tex.Load(int3(int2(texCoord.x,     texCoord.y - y), 0));
    float3 i = tex.Load(int3(int2(texCoord.x + x, texCoord.y - y), 0));
    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    float3 upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;

    return upsample;
}

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
[numthreads(16, 16, 1)]
void cs_main(uint3 GlobalInvocationID : SV_DispatchThreadID)
{
    float2 texCoord = float2(GlobalInvocationID.x, GlobalInvocationID.y);
    float2 texCoordTarget = texCoord * 2.0f;

    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = 0.003f;
    float y = 0.003f;

    Output[texCoordTarget].rgb = tentFilter(input,texCoord);
}

#endif


