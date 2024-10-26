#pragma once

#include <core/types.h>
#include <vulkan/vulkan.h>

#define NUM_FRAMES 3

struct FrameData {
	VkCommandPool command_pool;
	VkCommandBuffer main_command_buffer;

	VkSemaphore swapchain_semaphore;
	VkSemaphore render_semaphore;
	VkFence main_fence;
};



class Renderer2 {
public:
    Renderer2();
	~Renderer2();

    void draw(VkImage swapchain_image);

private:

	VkQueue m_queue;
	uint32_t m_queue_family;

    FrameData frame;
};
