#pragma once

#include <core/types.h>
#include <vulkan/vulkan.h>

#define NUM_FRAMES 3

struct FrameData {
	VkSemaphore swapchain_semaphore;
	VkSemaphore render_semaphore;
	VkFence main_fence;
}

class Renderer2 {
public:
    Renderer2(GfxSwapchain* colorSc, GfxSwapchain* depthSc);

    void draw();

private:
    FrameData m_frames[NUM_FRAMES];
};
