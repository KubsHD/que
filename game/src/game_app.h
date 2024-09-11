#include "app.h"


#include <common/GraphicsAPI_Vulkan.h>

#include <core/input.h>
#include <core/physics.h>
#include <gfx/buffers.h>
#include <gfx/sky.h>
#include <core/systems/base_system.h>


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

	template<typename T>
	void change_scene();

private:
	void init_imgui();

	void init_game_world();


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

	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<PhysicsSystem> m_physics_system;
	std::unique_ptr<AudioSystem> m_audio_system;

	Scene* m_current_scene;

	VkSampler sampler;

	std::vector<System> m_systems;
};

