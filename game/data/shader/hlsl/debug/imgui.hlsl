
float4 UnpackUnorm4x8(uint packedValue)
{
    float x = (packedValue & 0xFF) / 255.0f;         // Extract 8 bits (lowest byte)
    float y = ((packedValue >> 8) & 0xFF) / 255.0f;  // Extract next 8 bits
    float z = ((packedValue >> 16) & 0xFF) / 255.0f; // Extract next 8 bits
    float w = ((packedValue >> 24) & 0xFF) / 255.0f; // Extract highest 8 bits
    
    return float4(x, y, z, w);
}

struct ImguiDrawData {
	float2 translation;
	float2 scale;
};

struct ImguiVertex {
	float2 pos : POSITION;
	float2 uv : TEXCOORD0;
	uint color : COLOR0;
};

struct VS_OUTPUT {
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
	float4 color : COLOR0;
};

#if COMPILE_VS

[[vk::binding(0, 0)]]
StructuredBuffer<ImguiVertex> vertices : register(t0);

[[vk::push_constant]]
ImguiDrawData drawData;

VS_OUTPUT vs_main(uint vertex_id : SV_VertexID)
{
	ImguiVertex _in = vertices[vertex_id];

	VS_OUTPUT o;
	o.uv = _in.uv;
	o.color = UnpackUnorm4x8(_in.color);
	o.pos = float4(_in.pos *  drawData.scale + drawData.translation, 0.0, 1.0);

	return o;
}

#endif

#if COMPILE_PS 

[[vk::binding(1, 0)]]
[[vk::combinedImageSampler]]
Texture2D texture;
[[vk::binding(1, 0)]]
[[vk::combinedImageSampler]]
SamplerState texture_sm;

float gammaToLinear(float gamma) {
    return gamma < 0.04045 ?
        gamma / 12.92 :
        pow(max(gamma + 0.055, 0.0) / 1.055, 2.4);
}

float4 ps_main(VS_OUTPUT _in) : SV_TARGET
{
	float4 color = texture.Sample(texture_sm, _in.uv);
    color.rgb *= color.a;

	color.rgb = float3(
		gammaToLinear(color.r),
		gammaToLinear(color.g),
		gammaToLinear(color.b)
	);

	return color;
}

#endif