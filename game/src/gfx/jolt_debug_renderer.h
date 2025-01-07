#pragma once

#include <core/types.h>
#include <glm/glm.hpp>
#include "rhi/gfx_device.h"

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Core/Core.h>

class Renderer2;

class JoltDebugRenderer: public JPH::DebugRenderer {
	void init(Renderer2* r2);
public:
	JoltDebugRenderer();

	struct BatchImpl : public JPH::RefTargetVirtual {
	private:
		int m_ref_count = 0;
	public:

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		BatchImpl(const Vertex* inVert, int inVertCount, const JPH::uint32* inIdx, int inIdxCount)
		{
			vertices.resize(inVertCount);
			memcpy(vertices.data(), inVert, inVertCount * sizeof(Vertex));

			indices.resize(inIdxCount);
			memcpy(indices.data(), inIdx, inIdxCount * sizeof(JPH::uint32));
		}

		void AddRef() override { m_ref_count++; }
		void Release() override
		{
			if (--m_ref_count == 0) {
				delete this;
			}
		}
	};

	void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

	void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow =ECastShadow::Off) override;

	void DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow =ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) override;

	void DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) override;

	Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override;

	Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount) override;

};