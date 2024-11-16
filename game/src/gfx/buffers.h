#pragma once
#include <common/xr_linear_algebra.h>
#include <glm/fwd.hpp>

namespace gfx {

	struct SceneData {
		glm::mat4 viewProj;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 lightMtx;
		glm::vec3 camPos;
	};

	struct InstanceData {
		glm::mat4 model;
		glm::mat4 modelInvTrans{};
	};

}