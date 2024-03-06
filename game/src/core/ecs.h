/*****************************************************************//**
 * \file   ecs.h
 * \brief  Entity Component System
 * 
 * \date   January 2023
 *********************************************************************/

#pragma once

#include <core/types.h>
#include <list>

class Entity;
class Collider;
enum class CollisionTag;

#define RTTI_COMPONENT_NAME(NAME) \
const char* get_class_name() override { \
	return #NAME; \
} \

/// <summary>
/// Base class for all components
/// </summary>
class Component {
	friend class Scene;
	friend class Entity;
public:
	Component();
	virtual ~Component() = default;

	virtual const char* get_class_name()
	{
		return "Component";
	}

	/// <summary>
	/// Pointer to the entity this component is attached to
	/// </summary>
	Entity* entity;

	/// <summary>
	/// Is the component enabled
	/// </summary>
	bool enabled = true;

	/// <summary>
	/// Initialize the component
	/// </summary>
	virtual void init();

	/// <summary>
	/// Update the component
	/// </summary>
	virtual void update() = 0;

	/// <summary>
	/// Render the component
	/// </summary>
	/// <param name="ren">Renderer reference</param>
	virtual void render();

	virtual void destroy();
};

/// <summary>
/// Base class for all Scenes
/// </summary>
class Scene {
public:
	Scene();
	virtual ~Scene();

	/// <summary>
	/// Create an entity
	/// </summary>
	/// <param name="name">Entity name</param>
	/// <returns></returns>
	Entity* create(String name, Entity* parent = nullptr);

	/// <summary>
	/// Get an entity by name
	/// </summary>
	/// <param name="name">name</param>
	/// <returns></returns>
	Entity* get(String name);

	// TODO: docs
	Vector<Entity*> get_all(String name);


	/// <summary>
	/// Remove an entity
	/// </summary>
	/// <param name="name">Pointer to entity</param>
	void remove(Entity* name);

	/// <summary>
	/// Get all entities
	/// </summary>
	/// <returns></returns>
	Vector<Entity*> get_entities() { return m_entities; };

	/// <summary>
	/// Update all colliders
	/// </summary>
	void update_collider_list();
	
	void register_collider(Collider*);
	void deregister_collider(Collider*);

	/// <summary>
	/// Initialize the scene
	/// </summary>
	virtual void init() = 0;

	/// <summary>
	/// Update the scene
	/// </summary>
	virtual void update();

	/// <summary>
	/// Render the scene
	/// </summary>
	virtual void render();

	/// <summary>
	/// Destroy the scene
	/// </summary>
	virtual void destroy();
private:

	/// <summary>
	/// List of entities
	/// </summary>
	Vector<Entity*> m_entities; 
	
	/// <summary>
	/// List of entities marked for deletion
	/// </summary>
	Vector<Entity*> m_entites_marked_for_deletion;

	/// <summary>
	/// List of colliders
	/// </summary>
	Vector<Collider*> _colliders;
public:
	bool collision_query_sphere(Collider* requestor, Vec2 point, float radius, CollisionTag tagToQueryFor);
	std::list<Collider*> collision_query_sphere_list(Collider* requestor, Vec2 point, float radius, CollisionTag tagToQueryFor);
	bool collision_query_sphere_result(Collider* requestor, Vec2 point, float radius, CollisionTag tagToQueryFor, Entity& hit);
};

/// <summary>
/// Base class for all entities
/// </summary>
class Entity final {

	friend class Scene;
	
public:
	Entity() : position(640,320) {};

	/// <summary>
	/// Entity name
	/// </summary>
	String id;

	/// <summary>
	/// Entity name
	/// </summary>
	String name;

	/// <summary>
	/// Is the entity flipped
	/// </summary>
	bool flip;

	/// <summary>
	/// Entity position
	/// </summary>
	Vec2 position;

	/// <summary>
	/// Entity rotation
	/// </summary>
	float facing_angle;

	/// <summary>
	/// Reference to the scene this entity is in
	/// </summary>
	Scene* world;

	/// <summary>
	/// Add a component to the entity
	/// </summary>
	/// <typeparam name="T">Component class</typeparam>
	/// <param name="comp">Component constructor</param>
	/// <returns>Instance of an added component</returns>
	template<typename T>
	T* add(T&& comp = T());

	/// <summary>
	/// Get a component from the entity
	/// </summary>
	/// <typeparam name="T">Component type to return</typeparam>
	/// <returns>Component reference</returns>
	template<typename T>
	T* get()
	{
		T* m = nullptr;

		for (auto& comp : m_components)
		{
			if (m = dynamic_cast<T*>(comp))
				return m;
		}

		//throw std::runtime_error("Component not found!");
		return nullptr;
	};

	/// <summary>
	/// How many components are attached to the entity
	/// </summary>
	/// <returns></returns>
	int get_component_count() {return m_components.size();}
	Vector<Component*> get_components() { return m_components; }
	Vector<Entity*> get_children() { return m_children; }
	void add_children(Entity* ent);
	void destroy();
	void update();
	void render();
	void destroy_child(Entity* ent);
	void remove_child(Entity* ent);
	Entity* parent;
private:
	Vector<Component*> m_components;
	Vector<Entity*> m_children;
};


template<typename T>
T* Entity::add(T&& comp /*= T()*/)
{
	T* instance = new T();

	*instance = std::forward<T>(comp);

	((Component*)instance)->entity = this;

	((Component*)instance)->init();
    
	// hack
	this->world->update_collider_list();

	m_components.push_back((Component*)instance);
	return instance;
}