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

struct GPUDrawPushConstants {
	row_major float4x4 model;
};



// vk::binding(x,y)
// where x is the binding slot
// and y is the descriptor set index
// so layout(set=y, binding=x) in GLSL	

[[vk::binding(0,0)]]
ConstantBuffer<SceneData> Scene;

