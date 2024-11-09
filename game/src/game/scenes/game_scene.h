#pragma once

#include <core/ecs.h>
#include <gfx/mesh.h>
#include <core/audio.h>

class GameScene : public Scene {
public:
	void init() override;


	void update() override;


	void render() override;


	void destroy() override;

private:
	// game stuff
	Model level_model;
	Model controller;

	Model skybox_cube;
	GPUImage blank_texture;

	Model test_cube;

	Vector<Model> models;

	std::shared_ptr<Sound> bgm;

	void load_saved_objects();
	void save_objects();
};