#pragma once
#include <common/xr_linear_algebra.h>
#include <glm/fwd.hpp>

#include "light.h"

#define MAX_LIGHTS 32

namespace gfx {

	struct SceneData {
		glm::mat4 viewProj;
		glm::mat4 view;
		glm::mat4 proj;
		glm::mat4 lightMtx;
		glm::vec3 camPos;
		gfx::PointLight pointLight[MAX_LIGHTS];
		uint32_t pointLightCount;
		gfx::SpotLight spotLight[MAX_LIGHTS];
		uint32_t spotLightCount;
	};

	struct InstanceData {
		glm::mat4 model;
		glm::mat4 modelInvTrans{};
	};

}