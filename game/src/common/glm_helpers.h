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

	// https://github.com/tdbe/openxr-vulkan-project-fundamentals/blob/d4d3d9ccf70b2c3b685c4eea4df4f755c59c29bf/src/Util.cpp
	inline glm::mat4 to_glm_projection(const XrFovf& fov, float nearZ = 0.01f, float farZ = 100.0f) {
		const float l = glm::tan(fov.angleLeft);
		const float r = glm::tan(fov.angleRight);
		const float d = glm::tan(fov.angleDown);
		const float u = glm::tan(fov.angleUp);

		const float w = r - l;
		const float h = d - u;

		glm::mat4 projectionMatrix;
		projectionMatrix[0] = { 2.0f / w, 0.0f, 0.0f, 0.0f };
		projectionMatrix[1] = { 0.0f, 2.0f / h, 0.0f, 0.0f };
		projectionMatrix[2] = { (r + l) / w, (u + d) / h, -(farZ + nearZ) / (farZ - nearZ), -1.0f };
		projectionMatrix[3] = { 0.0f, 0.0f, -(farZ * (nearZ + nearZ)) / (farZ - nearZ), 0.0f };
		return projectionMatrix;
	}
}