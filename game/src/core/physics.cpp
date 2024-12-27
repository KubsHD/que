#include "pch.h"

#include "physics.h"



#include <jolt/Physics/StateRecorderImpl.h>
#include "profiler.h"
#include <Jolt/Physics/Character/CharacterVirtual.h>

JPH::StateRecorderImpl impl;

PhysicsSystem::PhysicsSystem()
{

	m_allocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads. Typically
	// you would implement the JobSystem interface yourself and let Jolt Physics run on top
	// of your own job scheduler. JobSystemThreadPool is an example implementation.
	m_job_system.Init(
		JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

	// This is the max amount of rigid bodies that you can add to the physics system. If you try to
	// add more you'll get an error. Note: This value is low because this is a simple test. For a
	// real project use something in the order of 65536.
	const std::uint32_t cMaxBodies = 1024;

	// This determines how many mutexes to allocate to protect rigid bodies from concurrent access.
	// Set it to 0 for the default settings.
	const std::uint32_t cNumBodyMutexes = 0;

	// This is the max amount of body pairs that can be queued at any time (the
	// broad phase will detect overlapping body pairs based on their bounding
	// boxes and will insert them into a queue for the narrowphase). If you
	// make this buffer too small the queue will fill up and the broad phase
	// jobs will start to do narrow phase work. This is slightly less
	// efficient. Note: This value is low because this is a simple test. For a
	// real project use something in the order of 65536.
	const std::uint32_t cMaxBodyPairs = 1024;

	// This is the maximum size of the contact constraint buffer. If more
	// contacts (collisions between bodies) are detected than this number then
	// these contacts will be ignored and bodies will start interpenetrating /
	// fall through the world. Note: This value is low because this is a simple
	// test. For a real project use something in the order of 10240.
	const std::uint32_t cMaxContactConstraints = 1024;

	// Now we can create the actual physics system.
	m_system.Init(
		cMaxBodies,
		cNumBodyMutexes,
		cMaxBodyPairs,
		cMaxContactConstraints,
		broad_phase_layer_interface,
		object_vs_broadphase_layer_filter,
		object_vs_object_layer_filter);

	// A body activation listener gets notified when bodies activate and go to
	// sleep Note that this is called from a job so whatever you do here needs
	// to be thread safe.
	// Registering one is entirely optional.
	m_system.SetBodyActivationListener(&body_activation_listener);

	// A contact listener gets notified when bodies (are about to) collide, and
	// when they separate again. Note that this is called from a job so
	// whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	m_system.SetContactListener(&contact_listener);

	// The main way to interact with the bodies in the physics system is
	// through the body interface.  There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though
	// we're not planning to access bodies from multiple threads)
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	// Next we can create a rigid body to serve as the floor, we make a large
	// box Create the settings for the collision volume (the shape).
	// Note that for simple shapes (like boxes) you can also directly construct a BoxShape.
	JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 0.2f, 100.0f));

	// Create the shape
	JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you
	// can check floor_shape_result for
	// HasError() / GetError()


	//JPH::BodyCreationSettings floor_settings(
	//	floor_shape,
	//	JPH::RVec3(0.0, -1.0, 0.0),
	//	JPH::Quat::sIdentity(),
	//	JPH::EMotionType::Static,
	//	Layers::NON_MOVING);

	//// Create the actual rigid body
	//floor = body_interface.CreateBody(floor_settings);

	//body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

	//JPH::BodyCreationSettings obj_settings(
	//	new JPH::SphereShape(0.01f),
	//	JPH::RVec3(0, 10.8, 0),
	//	JPH::Quat::sIdentity(),
	//	JPH::EMotionType::Dynamic,
	//	Layers::MOVING);
	//obj = body_interface.CreateAndAddBody(obj_settings, JPH::EActivation::Activate);

	//body_interface.SetAngularVelocity(obj, JPH::Vec3(0.7f, -1.0f, 0.1f));

	//auto grav = m_system.GetGravity();
	m_system.OptimizeBroadPhase();
}

PhysicsSystem::~PhysicsSystem()
{
}

static int step = 0;

const float cDeltaTime = 1.0f / 60.0f;

void PhysicsSystem::update(float dt, entt::registry& reg)
{
	QUE_PROFILE;
	/*for (auto& obj : m_bodies)
	{
		if (m_system.GetBodyInterface().IsActive(obj)) {
			JPH::RVec3 position = m_system.GetBodyInterface().GetCenterOfMassPosition(obj.first);
			JPH::Vec3 velocity = m_system.GetBodyInterface().GetLinearVelocity(obj.first);
			obj.second = { position.GetX(), position.GetY(), position.GetZ() };
	
			step++;
		}
	}*/

	m_system.Update(cDeltaTime, 1, m_allocator.get(), &m_job_system);


}

JPH::BodyID PhysicsSystem::spawn_body(JPH::BodyCreationSettings settings, JPH::Vec3 initial_velocity /*= JPH::Vec3(0, 0, 0)*/)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	auto id = body_interface.CreateAndAddBody(settings, JPH::EActivation::Activate);
	body_interface.AddLinearVelocity(id, initial_velocity);

	m_bodies.push_back(id);

	

	return id;
}

glm::vec3 PhysicsSystem::get_body_position(JPH::BodyID bodyId)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	auto ppos = body_interface.GetPosition(bodyId);

	return glm::vec3(ppos.GetX(), ppos.GetY(), ppos.GetZ());
}

glm::quat PhysicsSystem::get_body_rotation(JPH::BodyID bid)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	auto rot = body_interface.GetRotation(bid);

	return glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
}

void PhysicsSystem::set_body_position(JPH::BodyID bid, glm::vec3 pos)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	body_interface.SetPosition(bid, JPH::to_jph(pos), JPH::EActivation::Activate);
}

void PhysicsSystem::set_body_rotation(JPH::BodyID bid, glm::quat rot)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	body_interface.SetRotation(bid, JPH::to_jph(rot), JPH::EActivation::Activate);
}

void PhysicsSystem::add_velocity(JPH::BodyID bid, glm::vec3 vel)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	body_interface.AddForce(bid, JPH::to_jph(vel));
}

std::vector<JPH::Body*> PhysicsSystem::overlap_sphere(glm::vec3 point, float radius)
{
	auto& bpq = m_system.GetBroadPhaseQuery();

	JPH::AllHitCollisionCollector<JPH::CollideShapeBodyCollector> collector;
	bpq.CollideSphere(JPH::to_jph(point), radius, collector);

	std::vector<JPH::Body*> bodies;

	for (const auto& hit : collector.mHits)
	{
		auto body = m_system.GetBodyLockInterface().TryGetBody(hit);
		bodies.push_back(body);
	}


	return bodies;
}

JPH::EMotionType PhysicsSystem::get_body_type(JPH::BodyID bodyId)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	return body_interface.GetMotionType(bodyId);
}

void PhysicsSystem::set_motion_type(JPH::BodyID id, JPH::EMotionType param2)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();
	body_interface.SetMotionType(id, param2, JPH::EActivation::Activate);
}

void PhysicsSystem::update_character_virtual(JPH::CharacterVirtual* m_internal_cc)
{
	m_internal_cc->Update(cDeltaTime,
		m_system.GetGravity(),
		m_system.GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		m_system.GetDefaultLayerFilter(Layers::MOVING),
		{},
		{},
		*m_allocator);
}

void PhysicsSystem::init_static()
{
	JPH::Trace = TraceImpl;
	JPH::RegisterDefaultAllocator();
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();
}

JPH_SUPPRESS_WARNINGS



void TraceImpl(const char* inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
#if defined(XR_OS_WINDOWS)
	OutputDebugString(buffer);
#endif
	std::cout << buffer << std::endl;
}

bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint32_t inLine)
{
	// Print to the TTY
	std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << std::endl;

	// Breakpoint
	return true;
}

bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const
{
	switch (inObject1)
	{
	case Layers::NON_MOVING:
		return inObject2 == Layers::MOVING; // Non moving only collides with moving
	case Layers::MOVING:
		return true; // Moving collides with everything
	default:
		JPH_ASSERT(false);
		return false;
	}
}

BPLayerInterfaceImpl::BPLayerInterfaceImpl()
{
	// Create a mapping table from object to broad phase layer
	mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
	mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
}

uint32_t BPLayerInterfaceImpl::GetNumBroadPhaseLayers() const
{
	return BroadPhaseLayers::NUM_LAYERS;
}

JPH::BroadPhaseLayer BPLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
{
	JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
	return mObjectToBroadPhase[inLayer];
}

const char* BPLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
{
	switch ((JPH::BroadPhaseLayer::Type)inLayer)
	{
	case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
	case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
	default:													JPH_ASSERT(false); return "INVALID";
	}
}

bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const
{
	switch (inLayer1)
	{
	case Layers::NON_MOVING:
		return inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::MOVING:
		return true;
	default:
		JPH_ASSERT(false);
		return false;
	}
}

JPH::ValidateResult MyContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
{
	std::cout << "Contact validate callback" << std::endl;

	// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
	return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void MyContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
	std::cout << "A contact was added" << std::endl;
}

void MyContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
	std::cout << "A contact was persisted" << std::endl;
}

void MyContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
	std::cout << "A contact was removed" << std::endl;
}

void MyBodyActivationListener::OnBodyActivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData)
{
	std::cout << "A body got activated" << std::endl;
}

void MyBodyActivationListener::OnBodyDeactivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData)
{
	std::cout << "A body went to sleep" << std::endl;
}

glm::vec3 JPH::to_glm(Vec3 vec)
{
	return glm::vec3(vec.GetX(), vec.GetY(), vec.GetZ());
}

JPH::Vec3 JPH::to_jph(glm::vec3 vec)
{
	return JPH::Vec3(vec.x, vec.y, vec.z);
}

JPH::Quat JPH::to_jph(glm::quat q)
{
	return JPH::Quat(q.x, q.y, q.z, q.w);
}
