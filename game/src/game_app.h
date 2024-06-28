#include "app.h"


#include <common/GraphicsAPI_Vulkan.h>

#include <core/input.h>
#include <core/physics.h>
#include <gfx/buffers.h>
#include <gfx/sky.h>

class Renderer;
class AudioSystem;

class GameApp : public App
{
public:
	GameApp(GraphicsAPI_Type type);
	~GameApp();

	void run2();

	void init() override;


	void update(float dt) override;
	void render(FrameRenderInfo& info) override;
	void destroy() override;

private:
	void create_resources();
	void destroy_resources();
	void init_imgui();

	void init_game_world();

	Renderer* m_renderer;

	float m_viewHeightM = 1.5f;
	glm::vec3 player_pos = glm::vec3(0.0f);
	glm::vec3 m_forward;
	float m_speed = 0.1f;

	gfx::SceneData m_sceneDataCPU;
	gfx::Sky m_sky;

	VkBuffer m_sceneData;
	VkBuffer m_instanceData;

	GraphicsAPI::Pipeline m_pipeline;
	GraphicsAPI::Pipeline m_sky_render_pipeline;


	entt::registry m_registry;
	std::unique_ptr<PhysicsSystem> m_physics_system;
	std::unique_ptr<AudioSystem> m_audio_system;

	Model level_model;
	Model controller;

	Model skybox_cube;
	GraphicsAPI::Image skybox_image;
	GraphicsAPI::Image blank_texture;

	GraphicsAPI::Pipeline sky_pipeline;
	VkSampler sampler;
	Model test_cube;

	Vector<Model> models;

	std::shared_ptr<Sound> bgm;

	void load_saved_objects();
	void save_objects();
};