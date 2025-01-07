#if COMPILE_VS

float4 vs_main(uint VertexIndex: SV_VertexID) : SV_POSITION
{
    return float4(0, 0, 0, 0);
}

#endif

#if COMPILE_PS

float4 ps_main() : SV_TARGET
{
    return float4(0, 0, 0, 0);
}

#endif