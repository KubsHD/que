#define MAX_LIGHTS 32

struct PointLight {
	float3 position;
	float3 color;
	float intensity;
	float range;
};

struct SpotLight {
	float3 position;
	float3 direction;
	float3 color;
	float intensity;
	float range;
	float angle;
};

struct SceneData {
	row_major float4x4 viewProj;
	row_major float4x4 view;
	row_major float4x4 proj;
	row_major float4x4 lightMtx;
	float3 camPos;
	PointLight pointLights[MAX_LIGHTS];
	uint pointLightCount;
	SpotLight spotLights[MAX_LIGHTS];
	uint spotLightCount;
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

// https://forums.unrealengine.com/t/the-math-behind-combining-bc5-normals/365189/2
// reading bc5 compressed normal maps
float4 UnpackNormalMap(float4 TextureSample)
{

    float2 NormalXY = TextureSample.rg;
	
    NormalXY = NormalXY * float2(2.0f, 2.0f) - float2(1.0f, 1.0f);
    float NormalZ = sqrt(saturate(1.0f - dot(NormalXY, NormalXY)));
    return float4(NormalXY.xy, NormalZ, 1.0f);
}