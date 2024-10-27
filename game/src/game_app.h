#include "app.h"


#include <common/GraphicsAPI_Vulkan.h>

#include <core/xr/xr_input.h>
#include <core/physics.h>
#include <gfx/buffers.h>
#include <gfx/sky.h>
#include <core/systems/base_system.h>


class PlayerComponent;
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


	

	gfx::SceneData m_sceneDataCPU;
	gfx::Sky m_sky;

	VkBuffer m_sceneData;
	VkBuffer m_instanceData;

	GraphicsAPI::Pipeline m_pipeline;
	GraphicsAPI::Pipeline m_sky_render_pipeline;

	entt::registry m_registry;

	std::shared_ptr<AssetSystem> m_asset_manager;
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<PhysicsSystem> m_physics_system;
	std::unique_ptr<AudioSystem> m_audio_system;

	PlayerComponent* m_current_player_component;
	Scene* m_current_scene = nullptr;

	VkSampler sampler;

	std::vector<System> m_systems;
};

