#include "im3d_common.hlsli"


[[vk::push_constant]]
DrawData Data;

#if COMPILE_VS

VS_OUTPUT vs_main(uint vertex_id : SV_VertexID) 
{
	Im3dVertex _in = vertices[Data.offset + vertex_id];

	VS_OUTPUT ret;
	ret.m_color = UnpackUnorm4x8(_in.Color).abgr;
	ret.m_color.a *= smoothstep(0.0, 1.0, _in.Position.w / kAntialiasing);
	ret.m_size = max(_in.Position.w, kAntialiasing);
	ret.m_position = mul(float4(_in.Position.xyz, 1.0), Data.viewProj);
	return ret;
}

#endif

#if COMPILE_GS 

[maxvertexcount(4)]
void gs_main(line VS_OUTPUT _in[2], inout TriangleStream<VS_OUTPUT> out_)
{
	float2 pos0 = _in[0].m_position.xy / _in[0].m_position.w;
	float2 pos1 = _in[1].m_position.xy / _in[1].m_position.w;
		
	float2 dir = pos0 - pos1;
	dir = normalize(float2(dir.x, dir.y * Data.viewport.y / Data.viewport.x)); // correct for aspect ratio
	float2 tng0 = float2(-dir.y, dir.x);
	float2 tng1 = tng0 * _in[1].m_size / Data.viewport;
	tng0 = tng0 * _in[0].m_size / Data.viewport;
		
	VS_OUTPUT ret;
			
	// line start
	ret.m_size = _in[0].m_size;
	ret.m_color = _in[0].m_color;
	ret.m_uv = float2(0.0, 0.0);
	ret.m_position = float4((pos0 - tng0) * _in[0].m_position.w, _in[0].m_position.zw); 
	ret.m_edgeDistance = -_in[0].m_size;
	out_.Append(ret);
	ret.m_position = float4((pos0 + tng0) * _in[0].m_position.w, _in[0].m_position.zw);
	ret.m_edgeDistance = _in[0].m_size;
	out_.Append(ret);
			
	// line end
	ret.m_size = _in[1].m_size;
	ret.m_color = _in[1].m_color;
	ret.m_uv = float2(1.0, 1.0);
	ret.m_position = float4((pos1 - tng1) * _in[1].m_position.w, _in[1].m_position.zw);
	ret.m_edgeDistance = -_in[1].m_size;
	out_.Append(ret);
	ret.m_position = float4((pos1 + tng1) * _in[1].m_position.w, _in[1].m_position.zw);
	ret.m_edgeDistance = _in[1].m_size;
	out_.Append(ret);
}

#endif

#if COMPILE_PS

float4 ps_main(VS_OUTPUT vData) : SV_TARGET
{
    float4 ret = vData.m_color;
		
	float d = abs(vData.m_edgeDistance) / vData.m_size;
	d = smoothstep(1.0, 1.0 - (2.0f / vData.m_size), d);
	ret.a *= d;
		
	return ret;
}

#endif