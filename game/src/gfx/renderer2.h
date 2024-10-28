#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>
#include <common/deletion_queue.h>

#include "rhi/vk_descriptor.h"
#include "rhi/gfx_device.h"
#include "vertex.h"
#include "rhi/gfx_swapchain.h"
#include "buffers.h"

struct FrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;

	VkSemaphore swapchain_semaphore;
	VkSemaphore render_semaphore;
	VkFence main_fence;
};


struct GPUMeshBuffer {
	GPUBuffer vertex_buffer;
	GPUBuffer index_buffer;
	uint32_t index_count;
};


class Renderer2 {
public:
    Renderer2(VkFormat color_format);
	~Renderer2();

    void draw(Swapchain& swp, int image_index);

	GPUMeshBuffer upload_mesh(std::vector<uint32_t> indices, std::vector<Vertex2> vertices);

private:

	void draw_internal(VkCommandBuffer cmd);

	GPUMeshBuffer test;

	VkFormat m_color_format;

	VkQueue m_queue;
	uint32_t m_queue_family;

    FrameData frame;

	// desc
	DescriptorAllocator global_descriptor_allocator;

	VkDescriptorSetLayout unlit_pipeline_desc_layout;

	DeletionQueue main_deletion_queue;

	VkPipeline pip;
	VkPipelineLayout layout;

	void create_pipelines();
	void create_descriptors();

	// buffers
	VkDescriptorSet scene_data_set;
	gfx::SceneData m_scene_data_cpu{};
	GPUBuffer m_scene_data_gpu;

	VkDescriptorSet instance_data_set;
	GPUBuffer m_instance_data_gpu;
	gfx::InstanceData m_instance_data_cpu{};
};
