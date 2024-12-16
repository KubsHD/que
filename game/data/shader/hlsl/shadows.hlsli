[[vk::binding(1,0)]]
[[vk::combinedImageSampler]]
Texture2D shadow_map : register(t2);

[[vk::binding(1,0)]]
[[vk::combinedImageSampler]]
SamplerState shadow_map_sm : register(s2);

// https://github.com/SaschaWillems/Vulkan/blob/master/shaders/hlsl/shadowmapping/scene.vert
static const float4x4 biasMat = float4x4(
	0.5, 0.0, 0.0, 0.5,
	0.0, 0.5, 0.0, 0.5,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0);

float ShadowCalculation(float4 fragPosLightSpace)
{
    float4 shadowCoord = fragPosLightSpace / fragPosLightSpace.w;

    if (shadowCoord.z > 1.0)
        return 0.0;

    float shadow = 1.0;
    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0)
    {
        float dist = shadow_map.Sample(shadow_map_sm, shadowCoord.xy).r;
        if (shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = 0.1;
        }
    }

    return shadow;
}
