struct SceneData {
	float4x4 viewProj;
	float4x4 view;
	float4x4 proj;
	float3 camPos;
};

struct InstanceData {
	float4x4 model;
	float4x4 modelInvTrans;
};

[[vk::binding(0,0)]]
ConstantBuffer<SceneData> Scene;

[[vk::binding(1,0)]]
ConstantBuffer<InstanceData> Instance;


struct VertexInput
{
    [[vk::location(0)]] float3 position : POSITION;
    [[vk::location(1)]] float3 normal : NORMAL;
    [[vk::location(2)]] float3 tangent : TANGENT;
    [[vk::location(3)]] float3 bitangent : BITANGENT;
    [[vk::location(4)]] float2 texCoord : TEXCOORD;
};