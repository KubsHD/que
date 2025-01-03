#pragma once

#include <Jolt/Jolt.h>

#pragma region JOLT_BOILERPLATE

// Disable common warnings triggered by Jolt, you can use JPH_SUPPRESS_WARNING_PUSH / JPH_SUPPRESS_WARNING_POP to store and restore the warning state
JPH_SUPPRESS_WARNINGS


// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char* inFMT, ...);

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine);;

#endif // JPH_ENABLE_ASSERTS

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Layers
{
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool					ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override;
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr uint32_t NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl();

	virtual uint32_t					GetNumBroadPhaseLayers() const override;

	virtual JPH::BroadPhaseLayer			GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override;
};

// An example contact listener
class MyContactListener : public JPH::ContactListener
{
public:
	// See: ContactListener
	virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;

	virtual void			OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

	virtual void			OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

	virtual void			OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;
};

// An example activation listener
class MyBodyActivationListener : public JPH::BodyActivationListener
{
public:
	virtual void		OnBodyActivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override;

	virtual void		OnBodyDeactivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override;
};

#pragma endregion JOLT_BOILERPLATE

namespace JPH {
	glm::vec3 to_glm(JPH::Vec3 vec);
	JPH::Vec3 to_jph(glm::vec3 vec);
	JPH::Quat to_jph(glm::quat q);

	class CharacterVirtual;
}

class PhysicsSystem {
public:
	PhysicsSystem();
	~PhysicsSystem();

	static void init_static();

	void update(float dt, entt::registry& reg);
	JPH::BodyID spawn_body(JPH::BodyCreationSettings settings, JPH::Vec3 initial_velocity = JPH::Vec3(0, 0, 0));

	glm::vec3 get_body_position(JPH::BodyID bodyId);
	glm::quat get_body_rotation(JPH::BodyID bid);

	//Transform get_body_transform();

	void set_body_position(JPH::BodyID bid, glm::vec3 pos);
	void set_body_rotation(JPH::BodyID bid, glm::quat rot);

	void add_velocity(JPH::BodyID bid, glm::vec3 vel);

	glm::vec3 get_gravity();

	std::vector<JPH::Body*> overlap_sphere(glm::vec3 point, float radius);

	JPH::EMotionType get_body_type(JPH::BodyID bodyId);
	glm::vec3 obj_pos;

	JPH::PhysicsSystem* get_system() { return &m_system; }

private:
	JPH::PhysicsSystem m_system;
	std::unique_ptr<JPH::TempAllocatorImpl> m_allocator;
	JPH::JobSystemThreadPool m_job_system;

	JPH::Body* floor;
	JPH::BodyID obj;

	std::vector<JPH::BodyID> m_bodies;

	BPLayerInterfaceImpl broad_phase_layer_interface;

	ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl object_vs_object_layer_filter;

	MyContactListener contact_listener;
	MyBodyActivationListener body_activation_listener;
public:
	void set_motion_type(JPH::BodyID id, JPH::EMotionType param2);
	void update_character_virtual(JPH::CharacterVirtual* m_internal_cc);
};