#pragma once

#include <core/types.h>
#include <vulkan/vulkan.h>
#include "rhi/vk_descriptor.h"
#include <common/deletion_queue.h>

#define NUM_FRAMES 3

struct FrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;

	VkSemaphore swapchain_semaphore;
	VkSemaphore render_semaphore;
	VkFence main_fence;
};



class Renderer2 {
public:
    Renderer2(VkFormat color_format);
	~Renderer2();

    void draw(VkImage swapchain_image);

private:

	VkFormat m_color_format;

	VkQueue m_queue;
	uint32_t m_queue_family;

    FrameData frame;

	// desc
	DescriptorAllocator global_descriptor_allocator;

	VkDescriptorSet unlit_pipeline_desc_set;
	VkDescriptorSetLayout unlit_pipeline_desc_layout;

	DeletionQueue main_deletion_queue;

	VkPipeline pip;
	VkPipelineLayout layout;

	void create_pipelines();
	void create_descriptors();
};
