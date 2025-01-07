#include "pch.h"

#include "jolt_debug_renderer.h"

void JoltDebugRenderer::init(Renderer2* r2)
{
}

JoltDebugRenderer::JoltDebugRenderer()
{
	DebugRenderer::Initialize();
}

void JoltDebugRenderer::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{

}

void JoltDebugRenderer::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow /*=ECastShadow::Off*/)
{

}

void JoltDebugRenderer::DrawGeometry(JPH::RMat44Arg inModelMatrix, const JPH::AABox& inWorldSpaceBounds, float inLODScaleSq, JPH::ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode /*= ECullMode::CullBackFace*/, ECastShadow inCastShadow /*=ECastShadow::On*/, EDrawMode inDrawMode /*= EDrawMode::Solid*/)
{

}

void JoltDebugRenderer::DrawText3D(JPH::RVec3Arg inPosition, const std::string_view& inString, JPH::ColorArg inColor /*= JPH::Color::sWhite*/, float inHeight /*= 0.5f*/)
{

}

JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount)
{
	return new BatchImpl(inTriangles->mV, inTriangleCount, nullptr, 0);
}

JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const JPH::uint32* inIndices, int inIndexCount)
{
	return new BatchImpl(inVertices, inVertexCount, inIndices, inIndexCount);
}