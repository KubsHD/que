#include "pch.h"

#include "editor.h"
#include <asset/asset_manager.h>

#include <core/engine_wrapper.h>

#include <gfx/renderer2.h>

static bool g_editor_running = true;

Editor::Editor()
{
	ed_platform = new EditorPlatform();
	ed_platform->init(reg);

	m_audio_system = new AudioSystem();

	AssetManager::Init(*m_audio_system, *ed_platform->get_renderer());
	ed_platform->get_renderer()->load_default_resources();

	g_engine.audio = m_audio_system;
	g_engine.asset = new AssetManager();
	g_engine.render = ed_platform->get_renderer();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	io.Fonts->Build();
	io.Fonts->SetTexID(0);
	io.DisplaySize = ImVec2(8, 8);
	io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	ImGui::StyleColorsDark();
}

Editor::~Editor()
{
	ed_platform->destroy();
	delete ed_platform;
}

void Editor::run()
{
	g_editor_running = true;

	while (g_editor_running)
	{
		g_editor_running = ed_platform->poll();
		ed_platform->render({});
	}
}

