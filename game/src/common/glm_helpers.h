#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <openxr/openxr.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <common/GraphicsAPI.h>
#include <common/xr_linear_algebra.h>

namespace glm {
	inline glm::quat to_glm(const XrQuaternionf& q)
	{
		return glm::quat{ q.w, q.x, q.y, q.z };
	}

	inline glm::vec3 to_glm(const XrVector3f& v)
	{
		return glm::vec3(v.x, v.y, v.z);
	}

	inline glm::vec2 to_glm(const XrVector2f& v)
	{
		return glm::vec2(v.x, v.y);
	}


	inline glm::mat4 to_glm(const XrPosef& p)
	{
		glm::mat4 orientation = glm::mat4_cast(to_glm(p.orientation));
		glm::mat4 translation = glm::translate(glm::mat4{ 1 }, to_glm(p.position));
		return translation * orientation;
	}

	inline glm::mat4 to_glm(const XrMatrix4x4f& xrMat)
	{
		return glm::mat4(
			xrMat.m[0], xrMat.m[1], xrMat.m[2], xrMat.m[3],
			xrMat.m[4], xrMat.m[5], xrMat.m[6], xrMat.m[7],
			xrMat.m[8], xrMat.m[9], xrMat.m[10], xrMat.m[11],
			xrMat.m[12], xrMat.m[13], xrMat.m[14], xrMat.m[15]
		);
	}

	// https://github.com/jherico/OpenXR-Samples/blob/master/src/examples/sdl2_gl_single_file_example_c.cpp
	inline XrFovf toTanFovf(const XrFovf& fov) {
		return { tanf(fov.angleLeft), tanf(fov.angleRight), tanf(fov.angleUp), tanf(fov.angleDown) };
	}

	inline glm::mat4 to_glm_projection(const XrFovf& fov, float nearZ = 0.01f, float farZ = 10000.0f) {
		auto tanFov = toTanFovf(fov);
		const auto& tanAngleRight = tanFov.angleRight;
		const auto& tanAngleLeft = tanFov.angleLeft;
		const auto& tanAngleUp = tanFov.angleUp;
		const auto& tanAngleDown = tanFov.angleDown;

		const float tanAngleWidth = tanAngleRight - tanAngleLeft;
		const float tanAngleHeight = (tanAngleDown - tanAngleUp);
		const float offsetZ = 0;

		glm::mat4 resultm{};
		float* result = &resultm[0][0];
		// normal projection
		result[0] = 2 / tanAngleWidth;
		result[4] = 0;
		result[8] = (tanAngleRight + tanAngleLeft) / tanAngleWidth;
		result[12] = 0;

		result[1] = 0;
		result[5] = 2 / tanAngleHeight;
		result[9] = (tanAngleUp + tanAngleDown) / tanAngleHeight;
		result[13] = 0;

		result[2] = 0;
		result[6] = 0;
		result[10] = -(farZ + offsetZ) / (farZ - nearZ);
		result[14] = -(farZ * (nearZ + offsetZ)) / (farZ - nearZ);

		result[3] = 0;
		result[7] = 0;
		result[11] = -1;
		result[15] = 0;

		return resultm;
	}
}