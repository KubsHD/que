#pragma once

#include <core/ecs.h>
#include <gfx/mesh.h>
#include <core/audio.h>

class DevScene : public Scene {
public:
	void init() override;


	void update() override;


	void render() override;


	void destroy() override;


};