#pragma once

#include <core/types.h>

#include <vulkan/vulkan.h>
#include <common/deletion_queue.h>

#include "rhi/vk_descriptor.h"
#include "rhi/gfx_device.h"
#include "vertex.h"
#include "rhi/gfx_swapchain.h"
#include "buffers.h"
#include "mat/mat_unlit.h"

class MeshComponent;

struct FrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;

	VkSemaphore swapchain_semaphore;
	VkSemaphore render_semaphore;
	VkFence main_fence;

	DeletionQueue deletion_queue;
};

struct GPUDrawPushConstants {
	glm::mat4 model;
};

struct GPUMeshBuffer {
	GPUBuffer vertex_buffer;
	GPUBuffer index_buffer;
	uint32_t index_count;
};

class Renderer2 {
public:
    Renderer2(Swapchain& swapchain_info, entt::registry& reg);
	~Renderer2();

    void draw(Swapchain& swp, int image_index,XrView view);

	GPUMeshBuffer upload_mesh(std::vector<uint32_t> indices, std::vector<Vertex2> vertices);

	void register_mesh(const MeshComponent* mc);

	DeletionQueue main_deletion_queue;

	VkFormat color_format;
	VkFormat depth_format;
private:
	void draw_internal(VkCommandBuffer cmd);

	GPUMeshBuffer test;

	VkQueue m_queue;
	uint32_t m_queue_family;

	// depth stuff
	GPUImage depth_image;

    FrameData frame;

	entt::registry& m_reg;

	void create_global_descriptors();
	void create_pipelines();
	void create_default_textures();

public:
	// desc
	DescriptorAllocator global_descriptor_allocator;

	// global scene data
	VkDescriptorSet scene_data_set;
	VkDescriptorSetLayout scene_data_set_layout;

	gfx::SceneData m_scene_data_cpu{};
	GPUBuffer m_scene_data_gpu;

	// default resources
	GPUImage texture_white;
	GPUImage texture_black;
	GPUImage texture_checker;

	VkSampler default_sampler_linear;
	VkSampler default_sampler_nearest;

	// materials
	MAT_Unlit mat_unlit;

};
