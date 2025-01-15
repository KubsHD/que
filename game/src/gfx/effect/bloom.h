#pragma once

#include <core/types.h>

#include <gfx/rhi/gfx_device.h>

class Renderer2;


class BloomEffect
{
public:
	void init(Renderer2* r2);
	void render(VkCommandBuffer cmd, VkImageView inout_texture);
	void update();
	void destroy();

private:
	GPUImage temp;

	Vector<GPUImage> mips;

	GPUPipeline bloom_downscale_pipeline;
	VkDescriptorSet bloom_set;
	VkDescriptorSetLayout bloom_set_layout;

	
};

