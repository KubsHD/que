#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include <openxr/openxr.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glm {
	inline glm::quat to_glm(const XrQuaternionf& q)
	{
		return glm::quat{ q.w, q.x, q.y, q.z };
	}

	inline glm::vec3 to_glm(const XrVector3f& v)
	{
		return glm::make_vec3(&v.x);
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
}