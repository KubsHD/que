#pragma once

#include <core/ecs.h>

class MeshComponent : public Component {

public:
	void init() override;


	void update() override;


	void render() override;

};