#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <core/types.h>

#include <asset/mesh.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>

#include <core/audio.h>
#include <core/asset.h>
#include <core/physics.h>


struct core_uuid_component
{
	String uuid;
};

struct core_transform_component {
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 scale;

	glm::mat4 matrix;
	glm::quat rotation2;

	void calculate_matrix()
	{
		matrix = glm::mat4(1.0f);
		matrix = glm::translate(matrix, position);
		matrix *= glm::mat4(rotation);
		matrix = glm::scale(matrix, scale);
	}
};

struct physics_component {
	bool enabled;
	JPH::BodyID id;
};

struct core_mesh_component {
	Model* model;
};

struct scriptable_component {
	std::filesystem::path script;
};

struct controller_component {
	int index = 0;
	glm::vec3 last_pos;
	glm::vec3 current_pos;
	glm::vec3 vel;
};

enum CollisionState {
	IDLE,
	COLLIDE,
	TRIGGER
};

struct collider_component {
	CollisionState state;
	glm::vec3 bounds;
};

struct game_data_component {
	float tower_height;
	float score;

	// config
	float spawn_new_block_delay;
};

struct attach_component {
	entt::entity parent_entity;
};

struct game_state_component {
	float elapsed_time;
	float tower_height;
	bool is_game_over;
};
