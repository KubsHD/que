#pragma once

#include <core/ecs.h>
#include <core/engine_wrapper.h>

std::vector<Entity*> get_entities_from_collision_result(Scene* scn, std::vector<JPH::Body*> bodies);