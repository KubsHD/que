#include "app.h"

class GameApp : public App
{
public:
	GameApp(GraphicsAPI_Type type);
	~GameApp();

	void init() override;
	void update() override;
	void render(FrameRenderInfo& info) override;
	void destroy() override;

	void render_cube(XrPosef pose, XrVector3f scale, XrVector3f color);

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

	void* m_vertexBuffer = nullptr;
	void* m_indexBuffer = nullptr;
	void* m_uniformBuffer_Camera = nullptr;
	void* m_uniformBuffer_Normals = nullptr;
	void* m_vertexShader = nullptr, * m_fragmentShader = nullptr;
	void* m_pipeline = nullptr;
};