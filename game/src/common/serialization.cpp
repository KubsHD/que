#include "pch.h"
#include "serialization.h"

namespace ser {
	glm::vec3 vec3_deserialize(const nlohmann::json& j)
	{
		return glm::vec3(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>());
	}

	nlohmann::json vec3_serialize(const glm::vec3& vec)
	{
		return nlohmann::json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
	}
}