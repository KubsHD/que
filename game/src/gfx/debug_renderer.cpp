#include "pch.h"

#include "debug_renderer.h"

void DebugRenderer::init(Renderer2* r2)
{
	Vec4(0.5f, 0.5f, 0.5f, 1.0f);

	auto& ap = Im3d::GetAppData();
}

void DebugRenderer::render(VkCommandBuffer cmd)
{
}

void DebugRenderer::draw_box(Vec3 pos, Vec3 scale, Vec4 color /*= Colors::default_color*/)
{

}
