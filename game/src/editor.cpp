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

	AudioSystem audio_system;

	AssetManager::Init(audio_system, *ed_platform->get_renderer());
	ed_platform->get_renderer()->load_default_resources();

	//g_engine.audio = m_audio_system.get();
	//g_engine.physics = m_physics_system.get();
	//g_engine.reg = &m_registry;
	//g_engine.input = platform->input.get();

	g_engine.asset = new AssetManager();
	g_engine.render = ed_platform->get_renderer();

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
		ed_platform->render();
	}
}

