#include "pch.h"

#include "menu_scene.h"

#include <game/components/mesh_component.h>
#include <game/templates/controller_template.h>
#include <gfx/renderer2.h>



void MenuScene::init()
{
	const auto player = create("player");
	auto pc = player->add<PlayerComponent>();

	// controllers
	game::tmpl::create_controller(*this, 0, pc);
	game::tmpl::create_controller(*this, 1, pc);
}

void MenuScene::update()
{
	Scene::update();
}

void MenuScene::render()
{
}

void MenuScene::destroy()
{
}