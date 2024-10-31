#include "pch.h"

#include "game_scene.h"
#include <core/asset.h>
#include "game_app.h"
#include <core/physics_util.h>
#include <game/components/mesh_component.h>
#include <common/serialization.h>
#include <game/templates/block_template.h>
#include <game/templates/controller_template.h>

#include <game/components/interactable/speaker_interactable.h>

void GameScene::init()
{
	bgm = engine.asset->load_sound("audio/background_music_1.mp3");

	level_model = engine.asset->load_model("level/testlevel.gltf");
	skybox_cube = engine.asset->load_model("cube.gltf");
	controller =  engine.asset->load_model_json("models/meta/model_controller_left.model");
	skybox_image =engine.asset->load_image("apartment.hdr", TT_HDRI);
	test_cube =   engine.asset->load_model_json("models/blocks/brick/Cube.model");

	engine.audio->play_sound(*bgm);

	for (auto& mesh : level_model.meshes)
	{
		JPH::BodyCreationSettings obj_settings(
			core::physics::create_mesh_shape(mesh),
			JPH::RVec3(0, -1.0f, 0),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Static,
			Layers::NON_MOVING);

		obj_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
		obj_settings.mMassPropertiesOverride.mMass = 100.0f;
		obj_settings.mMassPropertiesOverride.mInertia = JPH::Mat44::sIdentity();

		engine.physics->spawn_body(obj_settings);
	}

	const auto player = create("player");
	auto pc = player->add<PlayerComponent>();

	const auto entity = create("test");
	entity->position = glm::vec3(0, -1, 0);
	entity->add<MeshComponent>(MeshComponent(&level_model));

	//// controllers
	game::tmpl::create_controller(*this, controller, 0, pc);
	game::tmpl::create_controller(*this, controller, 1, pc);
	game::tmpl::create_block(*this, glm::vec3(0, 2, 0), &test_cube, nullptr);

	// load saved objects
	load_saved_objects();
}

void GameScene::update()
{
	Scene::update();
}

void GameScene::render()
{
}

void GameScene::destroy()
{
}

void GameScene::load_saved_objects()
{
	auto so = engine.asset->read_json("saved_objects.json");

	models.reserve(so["objects"].size());

	for (auto obj : so["objects"])
	{
		auto pVector = &models;

		String modelPath = obj["model"];

		models.push_back(AssetManager::load_model_json(modelPath));

		JPH::RefConst<JPH::Shape> shape = nullptr;

		if (obj.contains("physics"))
			shape = core::physics::load_from_file(obj["physics"]);

		auto ent = game::tmpl::create_block(*this, ser::vec3_deserialize(obj["position"]), &models[models.size() - 1], shape);

		if (modelPath.find("glosnik") != std::string::npos)
			ent->add<SpeakerInteractableComponent>();
	}
}

void GameScene::save_objects()
{

}

