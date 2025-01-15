struct VSOutput {
    float4 position : SV_POSITION;
    float4 fragPosLightSpace : TEXCOORD2;
    float2 texCoord : TEXCOORD;
};

VSOutput vs_main(uint VertexIndex: SV_VertexID) {
    VSOutput o;

    return o;
}

#if COMPILE_PS


float4 ps_main(VSOutput input) : SV_TARGET {
    return 0;
}

#endif