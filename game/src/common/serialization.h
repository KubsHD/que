#pragma once

#include <lib/json.hpp>
#include <glm/fwd.hpp>

namespace ser {
	glm::vec3 vec3_deserialize(const nlohmann::json& j);
	nlohmann::json vec3_serialize(const glm::vec3& vec);
}