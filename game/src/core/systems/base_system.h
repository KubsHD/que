#pragma once

class System {
public:
	System() = default;

	virtual void init() = 0;

	virtual void update() = 0;

	virtual void destroy() = 0;
};