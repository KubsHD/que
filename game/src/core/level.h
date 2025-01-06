#pragma once

#include <core/types.h>
#include <gfx/mesh.h>
#include "ecs.h"

struct Object {
	Model model;
	glm::vec3 pos;
	glm::quat rot;
	glm::vec3 scale;
};

struct Level {
	std::string name;
	Vector<Object> objects;
	Scene* scene;
};

namespace core {
	Level load_level(String path, Scene* scene);
	void unload_level(Level& level);
}