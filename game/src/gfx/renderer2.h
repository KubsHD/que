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
#include "sky/sky.h"
#include "shadow_renderer.h"
#include "mat/mat_lit.h"
#include "buffers.h"
#include "light.h"
#include "effect/bloom.h"
#include "camera.h"

struct DebugRenderer;
class MeshComponent;

struct RenderTarget {
	GPUImage image;
	Vec2 size;
	VkFormat format;
};

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
	Renderer2(Swapchain& swp, entt::registry& reg);
    Renderer2(RenderTarget rt_info, entt::registry& reg);
	~Renderer2();

	void load_default_resources();

	void update();


	void wait_for_frame();
	GPUImage acquire_image(vkb::Swapchain& swapchain);
    void draw(RenderTarget rt, CameraRenderData view);

	void draw_xr(RenderTarget rt, CameraRenderData crd);


	GPUMeshBuffer upload_mesh(std::vector<uint32_t> indices, std::vector<Vertex2> vertices);
	
	void register_mesh(MeshComponent* mc);
	void unregister_mesh(MeshComponent* param1);

	DeletionQueue main_deletion_queue;

	VkFormat color_format;
	VkFormat depth_format;

	gfx::Sky sky;

	DebugRenderer* debug;

	// camera
	void set_camera_position(glm::vec3 pos);
	Vec3 get_camera_position();
	void present(VkSwapchainKHR swp);
private:
	void init_internal(RenderTarget rt_info, entt::registry& reg);
	void draw_internal(RenderTarget rt, CameraRenderData crd);

	VkQueue m_queue;
	uint32_t m_queue_family;


    FrameData frame;

	entt::registry& m_reg;

	void create_global_descriptors();
	void create_pipelines();
	void create_default_textures();

	ShadowRenderer m_shadow_renderer;

	glm::vec3 m_camera_position;
public:
	GPUImage depth_prepass;

	GPUImage offscren_color;
	GPUImage depth_image;

	GPUImage offscreen_tonemapped;

	// bloom
	BloomEffect bloom;

	// desc
	DescriptorAllocator global_descriptor_allocator;

	// global scene data
	VkDescriptorSet scene_data_set;
	VkDescriptorSetLayout scene_data_set_layout;

	gfx::SceneData m_scene_data_cpu{};
	GPUBuffer m_scene_data_gpu;

	Map<String, gfx::PointLight> m_point_lights;
	Map<String, gfx::SpotLight> m_spot_lights;


	// default resources
	GPUImage texture_white;
	GPUImage texture_black;
	GPUImage texture_checker;
	GPUImage texture_normal;

	VkSampler default_sampler_linear;
	VkSampler default_sampler_linear_clamp;

	VkSampler default_sampler_nearest;

	// materials
	MAT_Unlit mat_unlit;
	MAT_Lit mat_lit;
};
