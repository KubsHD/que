#pragma once

#include <gfx/rhi/gfx_device.h>

struct SkyIntermiedatePipeline {
	GPUPipeline pipeline;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSet descriptor_set;
};