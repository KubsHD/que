#include "pch.h"

#include "editor.h"
#include <asset/asset_manager.h>

#include <core/engine_wrapper.h>

#include <gfx/renderer2.h>
#include <lib/imgui/imgui_impl_sdl3.h>
#include <gfx/debug_renderer.h>

#define SDL_CALL(x) if (x == false) { LOG_INFO("SDL Error: %s\n", SDL_GetError()); }

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


}

Editor::~Editor()
{
	ed_platform->destroy();
	delete ed_platform;
}

void open_level_cb(void* userdata, const char* const* filelist, int filter)
{
	
}

void Editor::editor_ui()
{
	// main menu bar

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New"))
			{
				// new scene
			}
			if (ImGui::MenuItem("Open"))
			{
				SDL_DialogFileFilter filter{};
				filter.name = "Level Files";
				filter.pattern = "level";

				auto path = AssetManager::get_asset_dir().string();
				SDL_ShowOpenFileDialog(open_level_cb, nullptr, nullptr, &filter, 1, path.c_str(), false);
			}
			if (ImGui::MenuItem("Save"))
			{
				// save scene
			}
			if (ImGui::MenuItem("Exit"))
			{
				g_editor_running = false;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}


	// gizmo on 0,0,0

	Im3d::Text(Im3d::Vec3(0, 0, 0), 128.0f, Im3d::Color_Black, Im3d::TextFlags_Default, "test");

	{
		static int gridSize = 20;
		const float gridHalf = (float)gridSize * 0.5f;

		Im3d::SetAlpha(1.0f);
		Im3d::SetSize(1.0f);
		Im3d::BeginLines();
		for (int x = 0; x <= gridSize; ++x)
		{
			Im3d::Vertex(-gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(0.0f, 0.0f, 0.0f));
			Im3d::Vertex(gridHalf, 0.0f, (float)x - gridHalf, Im3d::Color(0.1f, 0.1f, 0.1f));
		}
		for (int z = 0; z <= gridSize; ++z)
		{
			Im3d::Vertex((float)z - gridHalf, 0.0f, -gridHalf, Im3d::Color(0.0f, 0.0f, 0.0f));
			Im3d::Vertex((float)z - gridHalf, 0.0f, gridHalf, Im3d::Color(0.1f, 0.1f, 0.1f));
		}
		Im3d::End();
	}


	//ImGui::SetNextWindowPos(ImVec2(100, 500), ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);
	//if (ImGui::Begin("Assets"))
	//{
	//	// iterate over all assets in model directory all build a draggable list of them
	//	for (auto& p : std::filesystem::recursive_directory_iterator(AssetManager::get_asset_dir() / "models"))
	//	{
	//		if (p.path().extension() == ".model")
	//		{
	//			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	//			{
	//				ImGui::SetDragDropPayload("ASSET", p.path().string().c_str(), p.path().string().size() + 1);
	//				ImGui::Button(p.path().filename().string().c_str());
	//				ImGui::EndDragDropSource();
	//			}
	//		}
	//	}

	//	ImGui::End();
	//}


	// draw gizmo at 0, 0, 0



}

void Editor::run()
{
	g_editor_running = true;

	while (g_editor_running)
	{
		g_editor_running = ed_platform->poll();

		Camera cam;
		cam.position = Vec3(0, 0, -10);

		ImGui_ImplSDL3_NewFrame();

		ImGui::NewFrame();
		//ImGui::ShowDemoWindow();

		g_engine.render->debug->begin_frame();

		ed_platform->update();
		editor_ui();

		ImGui::Render();
		g_engine.render->debug->end_frame();
		ed_platform->render(cam);
	}
}

