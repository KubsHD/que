#pragma once

#include <openxr/openxr.h>
#include <asset/mesh.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>



struct transform_component {
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;
};

struct physics_component {
	JPH::BodyID id;
};

struct mesh_component {
	Model model;
};

struct scriptable_component {
	std::filesystem::path script;
};

struct controller_component {
	int index = 0;
	glm::vec3 last_pos;
	glm::vec3 current_pos;
};