
#define MAX_VTX 1000000

struct VS_OUTPUT
{
	linear        float4 m_position     : SV_POSITION;
	linear        float4 m_color        : COLOR;
	linear        float2 m_uv           : TEXCOORD;
	noperspective float  m_size         : SIZE;
	noperspective float  m_edgeDistance : EDGE_DISTANCE;

    #if !COMPILE_PS
    [[vk::builtin("PointSize")]]
    float gl_PointSize : PSIZE;
    #endif
};

#define kAntialiasing 2.0

struct Im3dVertex
{
    float4 Position : POSITION;
    uint Color    : COLOR;
};

struct DrawData
{
    uint offset;
    row_major float4x4 viewProj;
    float2 viewport;
};

float4 UnpackUnorm4x8(uint packedValue)
{
    float x = (packedValue & 0xFF) / 255.0f;         // Extract 8 bits (lowest byte)
    float y = ((packedValue >> 8) & 0xFF) / 255.0f;  // Extract next 8 bits
    float z = ((packedValue >> 16) & 0xFF) / 255.0f; // Extract next 8 bits
    float w = ((packedValue >> 24) & 0xFF) / 255.0f; // Extract highest 8 bits
    
    return float4(x, y, z, w);
}

[[vk::binding(0, 0)]]
StructuredBuffer<Im3dVertex> vertices;