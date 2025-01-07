
#define MAX_VTX 1000000

struct Im3dVertex
{
    float4 Position : POSITION;
    float4 Color    : COLOR;
};

struct DrawData
{
    Im3dVertex buffer[MAX_VTX];
    float4x4 viewProj;
    float2 viewport;
};

