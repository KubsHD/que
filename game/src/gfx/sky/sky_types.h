#pragma once

#include <gfx/rhi/gfx_device.h>

struct SkyIntermiedatePipeline {
	GPUPipeline pipeline;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorSet descriptor_set;

	void clear(VkDevice device) {
		vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
		vkDestroyPipeline(device, pipeline.pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipeline.layout, nullptr);
	}
};