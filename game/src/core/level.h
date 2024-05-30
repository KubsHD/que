#pragma once

#include <core/types.h>
#include <asset/mesh.h>

struct Object {
	Model* model;
	glm::vec3 pos;
	glm::vec3 rot;
	glm::vec3 scale;
};

struct Level {
	std::string name;
	Vector<Object> objects;
};

namespace core {
	Level load_level(String path);
}