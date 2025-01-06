#include "pch.h"

#include "level.h"
#include "asset.h"

#include <game/components/mesh_component.h>
#include <common/serialization.h>

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

    }

    return lvl;
}
