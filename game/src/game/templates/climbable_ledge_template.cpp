#include "pch.h"

#include "enviroment_template.h"
#include "climbable_ledge_template.h"

#include <game/components/mesh_component.h>
#include <game/components/interactable/climbable_ledge_interactable.h>
#include <game/components/physics_component.h>


Entity* game::tmpl::create_climbable_ledge(Vec3 position, Scene& scn, PlayerComponent* pc)
{
	auto ent = scn.create("ledge");

	JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(0.2f, 0.2f, 0.2f));

	// Create the shape
	JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an 

	JPH::BodyCreationSettings obj_settings(
		floor_shape,
		JPH::to_jph(position),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Static,
		Layers::MOVING);

	ent->add<PhysicsComponent>(PhysicsComponent(obj_settings));

	ent->scale = Vec3(0.01f);
	ent->position = position;
	ent->add<ClimbableLedgeInteractable>();

	return ent;
}
