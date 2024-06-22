#pragma once

#include <entt/entt.hpp>


namespace game {
	namespace system {
		void on_attach_component_created(entt::registry& reg, entt::entity e);
		void on_attach_component_destroyed(entt::registry& reg, entt::entity e);
		void update_attach_system(entt::registry& reg);
	}
}