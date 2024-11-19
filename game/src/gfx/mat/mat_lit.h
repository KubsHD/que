#pragma once

#include <core/types.h>
#include <gfx/pipeline/builder.h>
#include <gfx/rhi/gfx_device.h>

class Renderer2;
class DescriptorAllocator;


struct MAT_Lit {
	GPUPipeline pipeline;
	VkDescriptorSetLayout material_layout;

	struct Resoruces {
		VkSampler sampler;
		
		GPUImage diffuse;
		GPUImage normal;
		GPUImage orm;
		GPUImage emission;
	};

	void create(Renderer2* ren);
	void clear(VkDevice device);

	MaterialInstance write(VkDevice device, const Resoruces& res, DescriptorAllocator* allocator);
};