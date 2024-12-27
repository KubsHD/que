/*****************************************************************//**
 * \file   ecs.h
 * \brief  Entity Component System
 * 
 * \date   January 2023
 *********************************************************************/

#pragma once

#include <core/types.h>
#include <list>
#include <core/engine_wrapper.h>
#include <unordered_map>

#include <core/rtti.h>

using EcsEntity = entt::entity;

class Entity;
class Collider;
enum class CollisionTag;
class GraphicsAPI;



#define DEFINE_COMPONENT(NAME) \
class NAME : public Component { \
public: \
	RTTI_COMPONENT_NAME(NAME) \
	NAME() = default; \


#define DEFINE_COMPONENT_DERIVED(NAME, BASE) \
class NAME : public BASE { \
public: \
	RTTI_COMPONENT_NAME(NAME) \
	NAME() = default; \

#define INSPECT(TYPE, NAME) \
TYPE NAME; \

#define REQUIRE_COMPONENT(NAME) \
assert(entity->get<NAME>() != nullptr); \




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

	template<typename T>
	Vector<T*> get_all_components_of_type();

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
	void draw_imgui();
private:

	/// <summary>
	/// List of entities
	/// </summary>
	Vector<Entity*> m_entities; 

	std::unordered_map<String, Entity*> m_uuid_entity_map;
	
	/// <summary>
	/// List of entities marked for deletion
	/// </summary>
	Vector<Entity*> m_entites_marked_for_deletion;

	/// <summary>
	/// List of colliders
	/// </summary>
	Vector<Collider*> _colliders;
public:
	engine_wrapper engine;

	template<typename T>
	T* get_first_component_of_type();

};



/// <summary>
/// Base class for all entities
/// </summary>
class Entity final {

	friend class Scene;
	
public:
	Entity() : position(0,0,0), rotation(glm::quat()), scale(1,1,1) {};

	/// <summary>
	/// Entity name
	/// </summary>
	String id;

	/// <summary>
	/// Entity name
	/// </summary>
	String name;

	/// <summary>
	/// Entity position
	/// </summary>
	Vec3 position;
	Vec3 local_position;

	Quat rotation;
	Quat local_rotation;

	Vec3 scale;
	Vec3 local_scale;

	/// <summary>
	/// Reference to the scene this entity is in
	/// </summary>
	Scene* scene;

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
	T* get();;

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
	void remove_child(Entity* ent);
	void set_parent(Entity* new_parent);
	Entity* parent;

	EcsEntity internal_entity;

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
    
	m_components.push_back((Component*)instance);
	return instance;
}

template<typename T>
T* Entity::get()
{
	T* m = nullptr;

	for (auto& comp : m_components)
	{
		if (m = dynamic_cast<T*>(comp))
			return m;
	}

	//throw std::runtime_error("Component not found!");
	return nullptr;
}

template<typename T>
Vector<T*> Scene::get_all_components_of_type()
{
	Vector<T*> components;

	for (auto& entity : m_entities)
	{
		auto comp = entity->get<T>();
		if (comp)
			components.push_back(comp);
	}

	return components;
}

template<typename T>
T* Scene::get_first_component_of_type()
{
	for (auto& entity : m_entities)
	{
		auto comp = entity->get<T>();
		if (comp)
			return comp;
	}
	return nullptr;
}