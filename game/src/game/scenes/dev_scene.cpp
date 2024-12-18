#include "pch.h"

#include "dev_scene.h"
#include <core/asset.h>
#include "game_app.h"
#include <core/physics_util.h>
#include <game/components/mesh_component.h>
#include <common/serialization.h>
#include <game/templates/block_template.h>
#include <game/templates/controller_template.h>

#include <gfx/renderer2.h>

#include <game/components/interactable/speaker_interactable.h>

static Model level_model;


void DevScene::init()
{
	
	const auto player = create("player");
	auto pc = player->add<PlayerComponent>();

	level_model = engine.asset->load_model("level/testlevel.gltf");

	const auto entity = create("test");
	entity->position = glm::vec3(0, -1, 0);
	entity->add<MeshComponent>(MeshComponent(&level_model));

	// controllers
	game::tmpl::create_controller(*this, 0, pc);
	game::tmpl::create_controller(*this, 1, pc);
}

void DevScene::update()
{
	Scene::update();
}

void DevScene::render()
{
}

void DevScene::destroy()
{
}
