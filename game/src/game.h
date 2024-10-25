#pragma once

#include <core/xr/xr_platform.h>

class Game {
public:
	Game() {};
	~Game() {};

	void run();

	OpenXRPlatform* platform;
};