#include "app.h"



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

	struct Mesh {
		VkBuffer* index_buffer;
		VkBuffer* vertex_buffer;
		uint32_t index_count;
	};

	GameApp(GraphicsAPI_Type type);
	~GameApp();

	void init() override;
	void update() override;
	void render(FrameRenderInfo& info) override;
	void destroy() override;

	void render_cube(XrPosef pose, XrVector3f scale, XrVector3f color);
	void render_mesh(XrVector3f pos, XrVector3f scale, XrQuaternionf rot, const Mesh& mesh);

	

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

	void* m_uniformBuffer_Camera = nullptr;
	void* m_uniformBuffer_Normals = nullptr;
	void* m_vertexShader = nullptr, * m_fragmentShader = nullptr;
	void* m_pipeline = nullptr;
};