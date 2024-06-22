#include "pch.h"

#include <Jolt/Core/StreamWrapper.h>

#include "physics_util.h"
#include "physics.h"
#include "asset.h"
#include <lib/json.hpp>

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

namespace nl = nlohmann;

glm::vec3 vec3_deserialize(const nlohmann::json& j) {
	return glm::vec3(j.at("x").get<float>(), j.at("y").get<float>(), j.at("z").get<float>());
}

nlohmann::json vec3_serialize(const glm::vec3& vec) {
	return nlohmann::json{ {"x", vec.x}, {"y", vec.y}, {"z", vec.z} };
}

JPH::BodyCreationSettings core::physics::load_from_file(String path)
{
	nl::json j = Asset::Instance->read_json(path);

	std::string fileName = j.at("file").get<std::string>();
	std::string type = j.at("type").get<std::string>();

	if (type == "sphere") {
		float radius = j.at("radius").get<float>();

		return JPH::BodyCreationSettings(
			new JPH::SphereShape(radius),
			JPH::RVec3(0, 0, 0),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Dynamic,
			Layers::MOVING);

		std::cout << "File: " << fileName << "\nType: " << type << "\nRadius: " << radius << std::endl;
	}
	else if (type == "box") {
		glm::vec3 size = vec3_deserialize(j.at("size"));

		return JPH::BodyCreationSettings(
			new JPH::BoxShape(JPH::to_jph(size)),
			JPH::RVec3(0, 0, 0),
			JPH::Quat::sIdentity(),
			JPH::EMotionType::Dynamic,
			Layers::MOVING);
	}
	else {
		std::cerr << "Unknown type: " << type << std::endl;
	}
}
