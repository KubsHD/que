#pragma once

#include <core/types.h>

#include <gfx/rhi/gfx_device.h>

class Renderer2;

struct BloomMip
{
	GPUImage texture;
	Vec2 size;
	glm::ivec2 int_size;
	VkDescriptorSet set;
	VkDescriptorSet upscale_set;
};

class BloomEffect
{
public:
	void init(Renderer2* r2);
	void render(VkCommandBuffer cmd, GPUImage input, GPUImage output);
	void update();
	void destroy();

private:
	Vector<BloomMip> mips;

	GPUImage temp;

	GPUPipeline bloom_downscale_pipeline;
	GPUPipeline bloom_upscale_pipeline;
	VkDescriptorSet bloom_set;
	VkDescriptorSetLayout bloom_set_layout;

	GPUPipeline bloom_render_pipeline;
	VkDescriptorSet bloom_render_set;
	VkDescriptorSetLayout bloom_render_set_layout;
	VkSampler sampler;

	int m_mip_count = 4;
};

