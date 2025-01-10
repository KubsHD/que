#include "pch.h"

#include "jolt_debug_renderer.h"

#if _DEBUG

Im3d::Vec3 joltToIm3d(const JPH::Vec3& v)
{
	return { v.GetX(), v.GetY(), v.GetZ() };
}

Im3d::Vec3 joltToIm3d(const JPH::Float3& v)
{
	return { v.x, v.y, v.z };
}

Im3d::Color joltToIm3d(const JPH::Color& color)
{
	return Im3d::Color(
		(float)color.r / 255.f,
		(float)color.g / 255.f,
		(float)color.b / 255.f,
		(float)color.a / 255.f);
}



void JoltDebugRenderer::init(Renderer2* r2)
{
}

JoltDebugRenderer::JoltDebugRenderer()
{
	DebugRenderer::Initialize();
}

void JoltDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
	Im3d::DrawLine(joltToIm3d(inFrom), joltToIm3d(inTo), 1.f, joltToIm3d(inColor));
}

void JoltDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow /*=ECastShadow::Off*/)
{
	Im3d::PushColor(joltToIm3d(inColor));
	Im3d::BeginTriangles();
	Im3d::Vertex(joltToIm3d(inV1));
	Im3d::Vertex(joltToIm3d(inV2));
	Im3d::Vertex(joltToIm3d(inV3));
	Im3d::End();
	Im3d::PopColor();
}



static int counter = 0;
void JoltDebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode /*= ECullMode::CullBackFace*/, ECastShadow inCastShadow /*=ECastShadow::On*/, EDrawMode inDrawMode /*= EDrawMode::Solid*/)
{
	counter++;

	const auto& b =
		*static_cast<const BatchImpl*>(inGeometry->mLODs[0].mTriangleBatch.GetPtr());
	for (std::size_t fi = 0; fi < b.indices.size(); fi += 3) {
		JPH::Float3 v0, v1, v2;
		(inModelMatrix * JPH::Vec3(b.vertices[b.indices[fi + 0]].mPosition)).StoreFloat3(&v0);
		(inModelMatrix * JPH::Vec3(b.vertices[b.indices[fi + 1]].mPosition)).StoreFloat3(&v1);
		(inModelMatrix * JPH::Vec3(b.vertices[b.indices[fi + 2]].mPosition)).StoreFloat3(&v2);

		if (inDrawMode == JPH::DebugRenderer::EDrawMode::Solid) {
			auto col = joltToIm3d(inModelColor);
			col.setA(0.9f);
			Im3d::PushColor(col);
			{
				auto& ctx = Im3d::GetContext();
				ctx.begin(Im3d::PrimitiveMode_Triangles);
				ctx.vertex(joltToIm3d(v0));
				ctx.vertex(joltToIm3d(v1));
				ctx.vertex(joltToIm3d(v2));
				ctx.end();
			}
			Im3d::PopColor();
		}
		else {
			float lineWidth = 0 != 0 ? 1.f : 3.f;
			Im3d::DrawLine(joltToIm3d(v0), joltToIm3d(v1), lineWidth, joltToIm3d(inModelColor));
			Im3d::DrawLine(joltToIm3d(v1), joltToIm3d(v2), lineWidth, joltToIm3d(inModelColor));
			Im3d::DrawLine(joltToIm3d(v2), joltToIm3d(v0), lineWidth, joltToIm3d(inModelColor));
		}
	}
}

void JoltDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor /*= JPH::Color::sWhite*/, float inHeight /*= 0.5f*/)
{

}

JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
{
	return new BatchImpl(inTriangles->mV, inTriangleCount * 3, nullptr, inTriangleCount * 3);
}

JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount)
{
	return new BatchImpl(inVertices, inVertexCount, inIndices, inIndexCount);
}

#endif