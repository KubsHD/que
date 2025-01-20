
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

Texture2D Input : register(t0);
SamplerState Input_sm : register(s0);

Texture2D BloomOutput : register(t1);
SamplerState BloomOutput_sm : register(s1);

RWTexture2D<float4> Output : register(u2);

#define bloomStrength 0.04
#define exposure 1.5

float3 ACESFilm(float3 x)
{
float a = 2.51f;
float b = 0.03f;
float c = 2.43f;
float d = 0.59f;
float e = 0.14f;
return saturate((x*(a*x+b))/(x*(c*x+d)+e));
}

[numthreads(16, 16, 1)]
void cs_main(uint3 GlobalInvocationID : SV_DispatchThreadID)
{
    float2 srcSize, dstSize;
    Output.GetDimensions(dstSize.x, dstSize.y);
    Input.GetDimensions(srcSize.x, srcSize.y);

    float2 bloomSrcSize;
    BloomOutput.GetDimensions(bloomSrcSize.x, bloomSrcSize.y);

    float2 texCoord = float2(GlobalInvocationID.x, GlobalInvocationID.y);
    texCoord = GlobalInvocationID.xy / dstSize;

    float3 hdrColor = Input.SampleLevel(Input_sm, texCoord, 0).rgb;

    float2 bloomCoord = GlobalInvocationID.xy / dstSize;
    float3 bloomColor = BloomOutput.SampleLevel(BloomOutput_sm, bloomCoord, 0).rgb;

    float3 result = lerp(hdrColor, bloomColor, bloomStrength);
  
    float3 exposed = result * exposure;
    float3 tonemapped = ACESFilm(exposed);
    float3 gammaCorrected = pow(tonemapped, 1.0 / 2.2);
    
    Output[GlobalInvocationID.xy] = float4(gammaCorrected, Input[GlobalInvocationID.xy].a);
}

#endif
