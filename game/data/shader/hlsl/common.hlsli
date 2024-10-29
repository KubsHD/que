struct SceneData {
	row_major float4x4 viewProj;
	row_major float4x4 view;
	row_major float4x4 proj;
	float3 camPos;
};

struct InstanceData {
	row_major float4x4 model;
	row_major float4x4 modelInvTrans;
};


// vk::binding(x,y)
// where x is the binding slot
// and y is the descriptor set index
// so layout(set=y, binding=x) in GLSL	

[[vk::binding(0,0)]]
ConstantBuffer<SceneData> Scene;

[[vk::binding(0,1)]]
ConstantBuffer<InstanceData> Instance;

[[vk::binding(1,1)]][[vk::combinedImageSampler]]
Texture2D tex_diffuse : register(t1);
[[vk::binding(1,1)]][[vk::combinedImageSampler]]
SamplerState tex_diffuse_sm : register(s1);

struct VertexInput
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float3 normal : NORMAL;
    [[vk::location(2)]] float3 tangent : TANGENT;
    [[vk::location(3)]] float3 bitangent : BITANGENT;
    [[vk::location(4)]] float2 texCoord : TEXCOORD;
};