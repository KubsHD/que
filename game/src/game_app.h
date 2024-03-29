#include "app.h"
#include <common/GraphicsAPI_Vulkan.h>
#include <core/input.h>
#include <entt/entt.hpp>
#include <core/physics.h>

#include <gfx/buffers.h>

//namespace glm {
//	// rot: {x,y,z, angle}
//	inline static glm::mat4 trs(glm::vec3 pos, glm::vec4 rot, glm::vec3 scale)
//	{
//		glm::mat4 m = glm::mat4(1.0f);
//		m = glm::scale(m, scale);
//		m = glm::rotate(m, rot.w, glm::vec3(rot.x, rot.y, rot.z));
//		m = glm::translate(m, pos);
//		return m;
//	}
//}

#include <gfx/sky.h>


class GameApp : public App
{
public:


	GameApp(GraphicsAPI_Type type);
	~GameApp();

	void init() override;
	void update(float dt) override;
	void render(FrameRenderInfo& info) override;
	void destroy() override;

	void GameApp::render_model(glm::vec3 pos, glm::vec3 scale, glm::quat rot, const Model& model);

	entt::registry& get_registry()
	{
		return m_registry;
	}

private:



	XrVector4f normals[6] = {
	{1.00f, 0.00f, 0.00f, 0},
	{-1.00f, 0.00f, 0.00f, 0},
	{0.00f, 1.00f, 0.00f, 0},
	{0.00f, -1.00f, 0.00f, 0},
	{0.00f, 0.00f, 1.00f, 0},
	{0.00f, 0.0f, -1.00f, 0} };

	void create_resources();
	void destroy_resources();


	float m_viewHeightM = 1.5f;

	gfx::SceneData m_sceneDataCPU;
	gfx::Sky m_sky;

	VkBuffer m_sceneData;
	VkBuffer m_instanceData;

	GraphicsAPI::Pipeline m_pipeline;
	GraphicsAPI::Pipeline m_sky_render_pipeline;

	entt::registry m_registry;
	std::unique_ptr<PhysicsSystem> m_physics_world;
};