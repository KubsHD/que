#include "pch.h"

#include "game.h"
#include <core/profiler.h>
#include <gfx/rhi/gfx_device.h>

void render(OpenXRPlatform::FrameRenderInfo info)
{

}

void Game::run()
{

	// init xrinstance
	// init gfxdevice with required extensions from xrinstance
	// init rest of xr stuff
	// create swapchain
	// main loop


//	m_start_time = std::chrono::high_resolution_clock::now();

	platform = new OpenXRPlatform();
	platform->init();

	platform->set_render_callback(render);

	bool running = true;
	while (running) {
		platform->poll();

		platform->render();

		QUE_PROFILE_FRAME();
	}

	platform->destroy();
}
