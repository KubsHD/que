#include "im3d_common.hlsli"

#if COMPILE_VS

[[vk::push_constant]]
DrawData Data;

VS_OUTPUT vs_main(uint vertex_id : SV_VertexID) 
{
	Im3dVertex _in = vertices[Data.offset + vertex_id];

	VS_OUTPUT ret;
	ret.m_color = UnpackUnorm4x8(_in.Color).abgr;
	ret.m_color.a *= smoothstep(0.0, 1.0, _in.Position.w / kAntialiasing);
	ret.m_size = max(_in.Position.w, kAntialiasing);
	ret.m_position = mul(Data.viewProj, float4(_in.Position.xyz, 1.0));
	ret.gl_PointSize = ret.m_size;

	return ret;
}

#endif

#if COMPILE_PS

float4 ps_main(VS_OUTPUT vData) : SV_TARGET
{
    float4 ret = vData.m_color;
		
	float d = length(vData.m_uv - float2(0.5, 0.5));
	d = smoothstep(0.5, 0.5 - (kAntialiasing / vData.m_size), d);
	ret.a *= d;
		
	return ret;
}


#endif