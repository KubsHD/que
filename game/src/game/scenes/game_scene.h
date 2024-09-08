#pragma once

#include <core/ecs.h>
#include <asset/mesh.h>
#include <core/audio.h>

class game_scene : public Scene {


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
	GraphicsAPI::Image skybox_image;
	GraphicsAPI::Image blank_texture;

	GraphicsAPI::Pipeline sky_pipeline;
	Model test_cube;

	Vector<Model> models;

	std::shared_ptr<Sound> bgm;

	void load_saved_objects();
	void save_objects();
};