#pragma once

#include <core/ecs.h>
#include <gfx/mesh.h>
#include <core/audio.h>

class MenuScene : public Scene {
public:
	void init() override;


	void update() override;


	void render() override;


	void destroy() override;

private:
};