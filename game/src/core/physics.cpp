#include "physics.h"





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
	JPH::BoxShapeSettings floor_shape_settings(JPH::Vec3(100.0f, 1.0f, 100.0f));

	// Create the shape
	JPH::ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	JPH::ShapeRefC floor_shape = floor_shape_result.Get(); // We don't expect an error here, but you
	// can check floor_shape_result for
	// HasError() / GetError()


	JPH::BodyCreationSettings floor_settings(
		floor_shape,
		JPH::RVec3(0.0, 1.0, 0.0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Static,
		Layers::NON_MOVING);

	// Create the actual rigid body
	floor = body_interface.CreateBody(floor_settings);

	body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);

	JPH::BodyCreationSettings obj_settings(
		new JPH::SphereShape(0.01f),
		JPH::RVec3(0, 10.8, 0),
		JPH::Quat::sIdentity(),
		JPH::EMotionType::Dynamic,
		Layers::MOVING);
	obj = body_interface.CreateAndAddBody(obj_settings, JPH::EActivation::Activate);

	body_interface.SetAngularVelocity(obj, JPH::Vec3(0.7f, -1.0f, 0.1f));

	m_system.OptimizeBroadPhase();
}

static int step = 0;

const float cDeltaTime = 1.0f / 60.0f;

void PhysicsSystem::update(float dt, entt::registry& reg)
{

	for (auto& obj : m_bodies)
	{
		if (m_system.GetBodyInterface().IsActive(obj.first)) {
			JPH::RVec3 position = m_system.GetBodyInterface().GetCenterOfMassPosition(obj.first);
			JPH::Vec3 velocity = m_system.GetBodyInterface().GetLinearVelocity(obj.first);
			obj.second = { position.GetX(), position.GetY(), position.GetZ() };
	
			step++;
		}
	}

	m_system.Update(cDeltaTime, 1, m_allocator.get(), &m_job_system);
}

JPH::BodyID PhysicsSystem::spawn_body(JPH::BodyCreationSettings settings, JPH::Vec3 initial_velocity /*= JPH::Vec3(0, 0, 0)*/)
{
	JPH::BodyInterface& body_interface = m_system.GetBodyInterface();

	auto id = body_interface.CreateAndAddBody(settings, JPH::EActivation::Activate);
	body_interface.AddLinearVelocity(id, initial_velocity);

	m_bodies.emplace(id, JPH::Vec3(0,0,0));

	return id;
}

glm::vec3 PhysicsSystem::get_body_position(JPH::BodyID bodyId)
{
	return glm::vec3(m_bodies[bodyId].GetX(), m_bodies[bodyId].GetY(), m_bodies[bodyId].GetZ());
}

void PhysicsSystem::init_static()
{
	JPH::RegisterDefaultAllocator();
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();
}
