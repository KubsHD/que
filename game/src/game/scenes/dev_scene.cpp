#include "pch.h"

#include "dev_scene.h"
#include <core/asset.h>
#include "game_app.h"
#include <core/physics_util.h>
#include <game/components/mesh_component.h>
#include <common/serialization.h>
#include <gfx/renderer2.h>

#include <game/templates/block_template.h>
#include <game/templates/controller_template.h>
#include <game/templates/climbable_ledge_template.h>

#include <game/components/interactable/speaker_interactable.h>
#include <game/components/character_controller.h>
#include <game/components/interactable/climbable_ledge_interactable.h>
#include <game/components/player/player_climb_manager_component.h>

#include <game/components/physics_component.h>
#include <core/level.h>
#include <game/components/light_component.h>

static Level level;
static Model lamp;

static std::shared_ptr<Sound> amb;

void DevScene::init()
{
	amb = g_engine.asset->load_sound("audio/amb_cave.wav");
	g_engine.audio->play_sound(amb);
	
	const auto player = create("player");
	player->position = glm::vec3(0, 1, 1);

	auto cc = player->add<CharacterController>();
	auto pc = player->add<PlayerComponent>(PlayerComponent(cc));
	player->add<PlayerClimbManager>();

	const auto floor = create("floor");
	floor->position = glm::vec3(0, -1, 0);

	JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));

	JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	JPH::ShapeRefC floor_shape = floor_shape_result.Get();

	JPH::BodyCreationSettings floor_settings(
		floor_shape,
		JPH::RVec3(0.0, -1.0, 0.0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Static,
		Layers::NON_MOVING);

	floor->add<PhysicsComponent>(PhysicsComponent(floor_settings));

	// controllers
	game::tmpl::create_controller(*this, 0, pc);
	game::tmpl::create_controller(*this, 1, pc);

	level = core::load_level("level/demo/demo.level", this);

	
	auto light = create("light");
	light->position = glm::vec3(-4.194f, 1.044f, 4.528f);
	light->rotation = glm::quat(0.707f, 0.7f, -0.06f, 0.437f);

	light->add<LightComponent>(LightComponent(LightType::Spot, glm::vec3(1, 1, 1), 205.0f, 100.0f));

	//light = create("light2");
	//light->position = glm::vec3(0, 10.0f, -2.0f);
	//light->rotation = glm::quat(glm::radians(glm::vec3(90.0f, 180.0f, 0.0f)));

	//light->add<LightComponent>(LightComponent(LightType::Spot, glm::vec3(1, 1, 1), 205.0f, 100.0f));
}

void DevScene::update()
{
	Scene::update();

	if (ImGui::Begin("Level"))
	{
		ImGui::Text("Level: %s", level.name.c_str());
		if (ImGui::Button("Reload"))
		{
			core::unload_level(level, this);
			level = core::load_level("level/demo/demo.level", this);
		}

		ImGui::End();
	}

	Im3d::Text(Im3d::Vec3(0, 1, 0), Im3d::TextFlags_Default, "asdadsadas");
}

void DevScene::render()
{
}

void DevScene::destroy()
{
}
