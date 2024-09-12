#include "pch.h"

#include "ecs.h"

#include <map>
#include <lib/imgui/imgui.h>
#include "uuid.h"
#include "components/components.h"

Entity* Scene::create(String name, Entity* parent)
{
	auto ent = new Entity();
	ent->scene = this;
	ent->name = name;
	ent->id = ID::get_uuid();
	ent->parent = nullptr;

	ent->internal_entity = engine.reg->create();

	engine.reg->emplace<core_transform_component>(ent->internal_entity, ent->position, ent->rotation, ent->scale);
	engine.reg->emplace<core_uuid_component>(ent->internal_entity, ent->id);


	if (parent != nullptr)
		parent->add_children(ent);
	else
		m_entities.push_back(ent);
	
	m_uuid_entity_map[ent->id] = ent;

	return ent;
}

// returns first found!!!!
Entity* Scene::get(String name)
{
	for (auto ent : m_entities)
		if (ent->name == name)
			return ent;

#if !__ANDROID__
	throw std::invalid_argument("Entity not found!");
#endif
}

void Scene::remove(Entity* name)
{
	if (std::find(m_entites_marked_for_deletion.begin(), m_entites_marked_for_deletion.end(), name) == m_entites_marked_for_deletion.end())
		m_entites_marked_for_deletion.push_back(name);
}

void Scene::init()
{
	m_entities.reserve(1000);
	_colliders.reserve(1000);
}

void Scene::update()
{
	auto query = engine.reg->view<core_uuid_component, core_transform_component>();

	for (const auto&& [e, uc, tc] : query.each())
	{
		Entity* found_entity = m_uuid_entity_map[uc.uuid];

		tc.position = found_entity->position;
		tc.rotation = found_entity->rotation;
		tc.scale = found_entity->scale;
	}

	for (int i = 0; i < m_entities.size(); i++)
	{
		auto ent = m_entities[i];

		if (ent->parent != nullptr)
		{
			ent->position = ent->parent->position;
			ent->rotation = ent->parent->rotation;
		}

		if (ent == nullptr)
			continue;

		ent->update();
	}

	// remove entities
	for (int i = m_entites_marked_for_deletion.size() - 1; i >= 0; --i) 
	{
		auto ent = m_entites_marked_for_deletion[i];

		for (auto comp : ent->m_components)
		{
			comp->destroy();
			delete comp;
		}

		m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), ent), m_entities.end());

		if (ent->parent != nullptr)
		{
			ent->parent->remove_child(ent);
		}

		m_entites_marked_for_deletion.pop_back();
		delete ent;
	}
}

template<typename T>
static void draw_component(T comp)
{
	// const Class *cls = GetClass<T>();

	//if (ImGui::TreeNode(cls->name))
	//{
	//	ImGui::LabelText("dasdadas", "dasdasdasd");
	//	ImGui::TreePop();
	//}
}

static void draw_entity(Entity* ent)
{
	if (ImGui::TreeNode(ent->id.c_str(), "%s : %s", ent->name.c_str(), ent->id.c_str()))
	{
		// draw position and rotation
		ImGui::Text("Position: %f %f %f", ent->position.x, ent->position.y, ent->position.z);
		ImGui::Text("Rotation: %f %f %f %f", ent->rotation.x, ent->rotation.y, ent->rotation.z, ent->rotation.w);
		ImGui::Text("Scale: %f %f %f", ent->scale.x, ent->scale.y, ent->scale.z);

		for (auto compo : ent->get_components())
		{
			ImGui::Text("%s", compo->get_class_name());

		//	////draw_component(compo);
		//	//auto cls = compo->get_class();
		//	//if (ImGui::TreeNode(cls->name.c_str()))
		//	//{
		//	//	for (auto field : cls->fields)
		//	//	{
		//	//		if (field.type == nullptr) break;

		//	//		if (field.type->hash == Hash("int"))
		//	//		{
		//	//			int* value = reinterpret_cast<int*>(&compo) + field.offset;
		//	//			ImGui::InputInt(field.name.c_str(), value);
		//	//		}
		//	//	}
		//	//	ImGui::TreePop();
		}

		for (auto child_ent : ent->get_children())
		{
			draw_entity(child_ent);
		}

		ImGui::TreePop();
	}
}

void Scene::render()
{

}

void Scene::destroy()
{
	for (int i = 0; i < m_entities.size(); i++)
	{
		auto ent = m_entities[i];

		ent->destroy();

		delete ent;
	}
}

void Scene::draw_imgui()
{
	if (ImGui::Begin("Hierarchy"))
	{
		for (auto ent : m_entities)
		{
			draw_entity(ent);
		}
		ImGui::End();
	}
}

bool Scene::collision_query_sphere_result(Collider* requestor, Vec2 point, float radius, CollisionTag tagToQueryFor, Entity& hit)
{
	return false;
}


std::list<Collider*> Scene::collision_query_sphere_list(Collider* requestor, Vec2 point, float radius, CollisionTag tagToQueryFor)
{
	std::list<Collider*> result;
	for (auto col : _colliders)
	{

	}

	return result;
}



bool Scene::collision_query_sphere(Collider* requestor, Vec2 point, float radius, CollisionTag tagToQueryFor)
{
	for (auto col : _colliders)
	{
	
	}

	return false;
}

Scene::Scene()
{

}

Scene::~Scene()
{

}

Component::Component()
{

}

void Component::init()
{

}

void Component::destroy()
{
}

void Entity::add_children(Entity* ent)
{
	ent->parent = this;
	m_children.push_back(ent);
}

void Entity::destroy()
{
	for (auto comp : m_components)
	{
		comp->destroy();

		delete comp;
	}

	for (auto c : m_children)
	{
		c->destroy();
	}
}

void Entity::update()
{
	for (auto comp : m_components)
	{
		if (comp->enabled)
			comp->update();
	}

	for (auto c : m_children)
		c->update();
}

void Entity::remove_child(Entity* ent)
{
	m_children.erase(std::remove(m_children.begin(), m_children.end(), ent), m_children.end());

	ent->position = this->position;
}

void Entity::set_parent(Entity* new_parent)
{
	if (parent != nullptr)
		parent->remove_child(this);

	parent = new_parent;

	if (new_parent != nullptr)
		new_parent->add_children(this);
}
