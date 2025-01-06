#include "pch.h"

#include "level.h"
#include "asset.h"

#include <game/components/mesh_component.h>
#include <game/components/physics_component.h>

#include <core/physics_util.h>
#include <common/serialization.h>
#include <game/templates/climbable_ledge_template.h>

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
		o.rot = glm::quat(ser::vec3_deserialize(obj["rotation"]));
		o.scale = ser::vec3_deserialize(obj["scale"]);

		// add the object to the scene
		auto e = scene->create(obj["model"]);
		e->position = o.pos;
		e->rotation = o.rot;
		e->scale = o.scale;

		e->add<MeshComponent>(MeshComponent(&lvl.objects.back().model));

		// create mesh collider

		if (!obj["col"])
			continue;

		auto shape = physics::create_mesh_shape(lvl.objects.back().model.meshes[0], o.scale);

		JPH::BodyCreationSettings settings(
			shape,
			JPH::to_jph(o.pos),
			JPH::to_jph(o.rot),
			JPH::EMotionType::Static,
			Layers::NON_MOVING);

		e->add<PhysicsComponent>(settings);
    }

	for (auto& ent : file["entities"])
	{
		auto ent_name = ent["name"].get<String>();

		if (ent_name._Starts_with("grabbable"))
		{
			game::tmpl::create_climbable_ledge(ser::vec3_deserialize(ent["position"]), *scene, nullptr);
		}
	}

    return lvl;
}
