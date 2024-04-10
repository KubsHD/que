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

struct rigidbody_component {
	JPH::BodyID id;
};

struct mesh_component {
	Model model;
};