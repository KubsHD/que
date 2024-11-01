#pragma once



#include <common/GraphicsAPI_Vulkan.h>
#include <core/xr/xr_input.h>
#include <entt/entt.hpp>
#include <core/physics.h>
#include <gfx/buffers.h>
#include <gfx/sky/sky.h>
#include <asset/mesh.h>
#include <app.h>

class MeshComponent;

class Renderer {
public:
	Renderer(std::shared_ptr<GraphicsAPI_Vulkan> gapi, std::vector<App::SwapchainInfo> colorFormats, std::vector<App::SwapchainInfo> depthFormats, entt::registry& reg);
	~Renderer();

	void render(glm::vec3 position, App::FrameRenderInfo& info);
	void render_model(glm::mat4 model_matrix, const Model& model);

	void register_mesh(const MeshComponent* mc);

private:

	struct MeshRenderData {
		Mesh mesh;
		Material mat;
	};


	void draw_sky(App::FrameRenderInfo& info);

	void create_engine_resources();
	void destroy_resources();

	std::shared_ptr<GraphicsAPI_Vulkan> m_graphicsAPI;

	gfx::SceneData m_sceneDataCPU;
	gfx::Sky m_sky;

	VkBuffer m_sceneData;
	VkBuffer m_instanceData;

	GPUModelConstant pushConst;

	int currently_drawn_object = 0;

	GraphicsAPI::Pipeline m_pipeline;
	GraphicsAPI::Pipeline m_unlit_pipeline;
	GraphicsAPI::Pipeline m_sky_pipeline;
	GraphicsAPI::Pipeline m_sky_render_pipeline;

	Model skybox_cube;
	GraphicsAPI::Image blank_texture;

	VkSampler sampler;

	std::vector<App::SwapchainInfo> m_colorSwapchainInfos = {};
	std::vector<App::SwapchainInfo> m_depthSwapchainInfos = {};

	entt::registry& m_reg;
};