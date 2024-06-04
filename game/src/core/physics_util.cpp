#include "pch.h"

#include <Jolt/Core/StreamWrapper.h>

#include "physics_util.h"

JPH::RefConst<JPH::Shape> core::physics::create_mesh_shape(Mesh m)
{
	JPH::VertexList vertices;
	JPH::IndexedTriangleList indices;

	for (auto vtx : m.vertices)
	{
		vertices.push_back({ vtx.x, vtx.y, vtx.z });
	}

	for (int i = 0; i < m.indices.size(); i+=3)
	{
		JPH::IndexedTriangle tri(m.indices[i], m.indices[i + 1], m.indices[i + 2]);
		indices.push_back(tri);
	}

	JPH::MeshShapeSettings mss(vertices, indices);
	JPH::Shape::ShapeResult result;
	JPH::MeshShape* ms = new JPH::MeshShape(mss, result);

	assert(!result.HasError());

	return ms;
}

JPH::RefConst<JPH::Shape> core::physics::create_convex_shape(Mesh m)
{
	JPH::Array<JPH::Vec3> points;

	for (auto vtx : m.vertices)
	{
		points.push_back({ vtx.x, vtx.y, vtx.z });
	}
	JPH::Shape::ShapeResult result;


	JPH::ConvexHullShapeSettings settings(points);
	settings.SetDensity(100.0f);

	auto cook_result = settings.Create();
	assert(cook_result.IsValid());

	//std::ofstream data("BLOCK.collision_c");
	//JPH::StreamOutWrapper stream_out(data);
	//JPH::Shape::ShapeToIDMap shape_to_id;
	//JPH::Shape::MaterialToIDMap material_to_id;
	//cook_result.Get()->SaveWithChildren(stream_out, shape_to_id, material_to_id);


	return cook_result.Get();
}
