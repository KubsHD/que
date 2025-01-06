#include "pch.h"

#include <Jolt/Core/StreamWrapper.h>

#include "physics_util.h"
#include "physics.h"
#include "asset.h"
#include <lib/json.hpp>
#include <common/serialization.h>
#include <asset/asset_manager.h>

JPH::RefConst<JPH::Shape> core::physics::create_mesh_shape(Mesh m, Vec3 scale)
{
	JPH::VertexList vertices;
	JPH::IndexedTriangleList indices;

	for (auto vtx : m.vertices)
	{
		vertices.push_back({ vtx.x * scale.x, vtx.y * scale.y, vtx.z * scale.z });
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

namespace nl = nlohmann;


JPH::RefConst<JPH::Shape> core::physics::load_from_file(String path)
{
	nl::json j = AssetManager::read_json(path);

	std::string fileName = j.at("file").get<std::string>();
	std::string type = j.at("type").get<std::string>();

	if (type == "sphere") {
		float radius = j.at("radius").get<float>();

		return new JPH::SphereShape(radius);


		std::cout << "File: " << fileName << "\nType: " << type << "\nRadius: " << radius << std::endl;
	}
	else if (type == "box") {
		glm::vec3 size = ser::vec3_deserialize(j.at("size"));

		return new JPH::BoxShape(JPH::to_jph(size));
	}
	else {
		std::cerr << "Unknown type: " << type << std::endl;
		return nullptr;
	}
}
