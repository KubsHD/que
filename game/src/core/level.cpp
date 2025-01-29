#include "pch.h"

#include "level.h"
#include "asset.h"

#include <game/components/mesh_component.h>
#include <game/components/physics_component.h>

#include <core/physics_util.h>
#include <common/serialization.h>
#include <game/templates/climbable_ledge_template.h>
#include <game/components/light_component.h>

Level core::load_level(String path, Scene* scene)
{
    Level lvl;

	lvl.scene = scene;

    auto file = g_engine.asset->read_json(path);
    
    // parse json
    lvl.name = file["name"];


	// preallocate object array
	lvl.objects.reserve(file["objects"].size());

    for (auto& obj : file["objects"])
    {
		// get the directory of the level file and add obj["model"] to it
		// this is because the model path is relative to the level file
        Path model_path(path);
		model_path = model_path.parent_path() / obj["model"].get<String>();

		lvl.objects.push_back(Object());

		auto& o = lvl.objects.back();

		o.model = g_engine.asset->load_model_json(model_path);
		o.pos = ser::vec3_deserialize(obj["position"]);

		auto angle = ser::vec3_deserialize(obj["rotation"]);

		o.rot = glm::quat(glm::radians(angle));
		o.scale = ser::vec3_deserialize(obj["scale"]);

		// add the object to the scene
		auto e = scene->create(obj["model"]);
		e->position = o.pos;
		e->rotation = o.rot;
		e->scale = o.scale;

		e->add<MeshComponent>(MeshComponent(&lvl.objects.back().model));

		lvl.entities.push_back(e);

		// create mesh collider

		if (!obj["col"])
			continue;

		auto shape = physics::create_mesh_shape(lvl.objects.back().model.meshes[0], o.scale);

		JPH::BodyCreationSettings asd{};

		e->add<PhysicsComponent>(PhysicsComponent(o.model.meshes[0]));
    }


	for (auto& ent : file["entities"])
	{
		auto ent_name = ent["name"].get<String>();

		if (ent_name._Starts_with("grabbable"))
		{
			game::tmpl::create_climbable_ledge(ser::vec3_deserialize(ent["position"]), *scene, nullptr);
		}

		if (ent_name._Starts_with("spot"))
		{
			auto light = scene->create("spot_light");
			light->position = ser::vec3_deserialize(ent["position"]);
			light->rotation = glm::quat(glm::radians(ser::vec3_deserialize(ent["rotation"])));
			light->add<LightComponent>(LightComponent(LightType::Spot, ser::vec3_deserialize(ent["color"]), ent["intensity"], ent["range"]));
		}

		if (ent_name._Starts_with("prop_lamp"))
		{
			auto prop = scene->create("prop_lamp");
			prop->position = ser::vec3_deserialize(ent["position"]);
			//prop->rotation = glm::quat(glm::radians(ser::vec3_deserialize(ent["rotation"])));
			//prop->scale = ser::vec3_deserialize(ent["scale"]);
			prop->scale = Vec3(0.01f);
			// rotate 180 degrees
			prop->rotation = glm::angleAxis(glm::radians(180.0f), Vec3(0, 1, 0));

			const String lamp_model_path = "models/env/lamp/env_industrial_light.model";

			prop->add<MeshComponent>(MeshComponent(lamp_model_path));
		}
	}

    return lvl;
}

void core::unload_level(Level& level, Scene* scene)
{
	for (auto& ent : level.entities)
	{
		scene->remove(ent);
	}

	level.entities.clear();

	for (auto& obj : level.objects)
	{
		g_engine.asset->unload_model(obj.model);
	}

	level.objects.clear();
}
