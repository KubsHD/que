#include "app.h"
#include <common/GraphicsAPI_Vulkan.h>
#include <core/input.h>
#include <entt/entt.hpp>
#include <core/physics.h>



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

class GameApp : public App
{
public:


	GameApp(GraphicsAPI_Type type);
	~GameApp();

	void init() override;
	void update(float dt) override;
	void render(FrameRenderInfo& info) override;
	void destroy() override;

	void render_model(XrVector3f pos, XrVector3f scale, XrQuaternionf rot, const Model& model);

	entt::registry& get_registry()
	{
		return m_registry;
	}

private:

	struct SceneData {
		XrMatrix4x4f viewProj;
		XrMatrix4x4f view;
		XrMatrix4x4f proj;
		XrVector3f camPos;
	};

	struct InstanceData {
		XrMatrix4x4f model{};
		XrMatrix4x4f modelInvTrans{};
	};


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

	SceneData m_sceneDataCPU;

	VkBuffer m_sceneData;
	VkBuffer m_instanceData;

	VkPipeline m_pipeline = nullptr;

	entt::registry m_registry;
	std::unique_ptr<PhysicsWorld> m_physics_world;
};