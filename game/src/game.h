#pragma once

#include <core/xr/xr_platform.h>

#include <gfx/buffers.h>
#include <gfx/sky.h>
#include <core/xr/xr_input.h>
#include <core/physics.h>
#include <core/systems/base_system.h>

class PlayerComponent;
class AudioSystem;

class Game {
public:
	Game();;
	~Game() {};

	void run();

	OpenXRPlatform* platform;

	template<typename T>
	void change_scene();

private:
	void update();

	std::chrono::steady_clock::time_point m_start_time;
	std::chrono::steady_clock::time_point m_last_time;
	float m_time;
	float m_delta_time;


	gfx::SceneData m_sceneDataCPU;
	gfx::Sky m_sky;

	VkBuffer m_sceneData;
	VkBuffer m_instanceData;

	GraphicsAPI::Pipeline m_pipeline;
	GraphicsAPI::Pipeline m_sky_render_pipeline;

	entt::registry m_registry;

	std::shared_ptr<AssetSystem> m_asset_manager;
	std::unique_ptr<Renderer2> m_renderer;
	std::unique_ptr<PhysicsSystem> m_physics_system;
	std::unique_ptr<AudioSystem> m_audio_system;

	PlayerComponent* m_current_player_component{};
	Scene* m_current_scene = nullptr;

	VkSampler sampler;
	void init_imgui();
};