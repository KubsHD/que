#include "pch.h"
#include "block_template.h"

#include <core/physics.h>
#include <core/physics_util.h>
#include <game/components.h>
#include <core/physics.h>
#include <core/physics_util.h>
#include <game/tags.h>
#include <game/components/speaker_interactable.h>

entt::entity game::tmpl::create_block(entt::registry& reg, PhysicsSystem& psys, glm::vec3 pos, Model& mod, JPH::RefConst<JPH::Shape> customShape)
{
	const auto ball = reg.create();

	reg.emplace<transform_component>(ball, glm::vec3{ 0.0f,5.0f,0.0f }, glm::quat(1, 0, 0, 0), glm::vec3{ 1,1,1 });
	reg.emplace<mesh_component>(ball, mod);

	
	auto shape = core::physics::create_convex_shape(mod.meshes[0]);
	shape->ScaleShape(JPH::Vec3(0.5f, 0.5f, 0.5f));
	
	if (customShape)
		customShape->GetMassProperties();

	JPH::BodyCreationSettings obj_settings(
		customShape != nullptr ? customShape : shape,
		JPH::to_jph(pos),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Dynamic,
		Layers::MOVING);


	if (customShape == nullptr)
	{
		obj_settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
		obj_settings.mMassPropertiesOverride.SetMassAndInertiaOfSolidBox(2.0f * JPH::Vec3(2.0f, 2.0f, 2.0f), 1000.0f);
	}
	reg.emplace<physics_component>(ball, true, psys.spawn_body(obj_settings, JPH::Vec3(0, 0.0f, 0.0f)));
	reg.emplace<pickupable_block>(ball);
	reg.emplace<speaker_interactable>(ball);

	return ball;
}
