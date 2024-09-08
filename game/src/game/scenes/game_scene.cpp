#include "pch.h"
//
//#include "game_scene.h"
//#include <core/asset.h>
//#include "game_app.h"
//
//void game_scene::init()
//{
//	bgm = engine->asset.load_sound("data/audio/background_music_1.mp3");
//
//	level_model = engine->asset.load_model("data/level/testlevel.gltf");
//	skybox_cube = engine->asset.load_model("data/cube.gltf");
//	controller = engine->asset.load_model_json("data/models/meta/model_controller_left.model");
//	skybox_image = engine->asset.load_image("data/apartment.hdr", TT_HDRI);
//	test_cube = engine->asset.load_model_json("data/models/blocks/brick/Cube.model");
//
//
//
//
//	m_audio_system->play_sound(*bgm);
//
//	for (auto& mesh : level_model.meshes)
//	{
//		JPH::BodyCreationSettings obj_settings(
//			core::physics::create_mesh_shape(mesh),
//			JPH::RVec3(0, -1.0f, 0),
//			JPH::Quat::sIdentity(),
//			JPH::EMotionType::Static,
//			Layers::NON_MOVING);
//
//		obj_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
//		obj_settings.mMassPropertiesOverride.mMass = 100.0f;
//		obj_settings.mMassPropertiesOverride.mInertia = JPH::Mat44::sIdentity();
//
//		m_physics_system->spawn_body(obj_settings);
//	}
//
//
//
//	const auto entity = m_registry.create();
//	m_registry.emplace<transform_component>(entity, glm::vec3{ 0.0f,-1.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 1.0f, 1.0f, 1.0f });
//	m_registry.emplace<mesh_component>(entity, level_model);
//
//	// controllers
//	game::tmpl::create_controller(m_registry, controller, 0);
//	game::tmpl::create_controller(m_registry, controller, 1);
//	game::tmpl::create_block(m_registry, *m_physics_system, glm::vec3(0, 2, 0), test_cube, nullptr);
//
//	// load saved objects
//	load_saved_objects();
//}
//
//void game_scene::update()
//{
//}
//
//void game_scene::render()
//{
//}
//
//void game_scene::destroy()
//{
//}
//
//void game_scene::load_saved_objects()
//{
//	auto so = engine->asset.read_json("data/saved_objects.json");
//
//	for (auto obj : so["objects"])
//	{
//		models.push_back(AssetSystem::load_model_json(obj["model"]));
//
//		//JPH::RefConst<JPH::Shape> shape = nullptr;
//
//		//if (obj.contains("physics"))
//		//	shape = core::physics::load_from_file(obj["physics"]);
//
//		//auto b = game::tmpl::create_block(m_registry, *m_physics_system, ser::vec3_deserialize(obj["position"]), models.back(), shape);
//		//m_registry.emplace<saveable>(b);
//	}
//}
//
//void game_scene::save_objects()
//{
//
//}
//
