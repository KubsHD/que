#pragma once

#include <entt/entt.hpp>

class System {
public:
	System(entt::registry& reg) : m_reg(reg) {};

	virtual void init() = 0;

	virtual void update() = 0;

	virtual void destroy() = 0;

protected:
	entt::registry& m_reg;
};