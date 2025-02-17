#include "pch.h"

#include "profiler.h"

#if XR_OS_WINDOWS && _DEBUG

static tracy::VkCtx* internal_ctx = nullptr;

tracy::VkCtx* tracy_get_ctx()
{
	return internal_ctx;
}

void tracy_init_ctx(VkPhysicalDevice physical_device, VkDevice device, VkQueue queue, VkCommandBuffer cmd)
{
	internal_ctx = TracyVkContext(physical_device, device, queue, cmd);
}

#endif