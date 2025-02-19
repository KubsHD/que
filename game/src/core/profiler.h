#pragma once

#include <chrono>

#define TIME_MEASURE_START \
	auto time_begin = std::chrono::high_resolution_clock::now();

#define TIME_MEASURE_END                                                                                                              \
	auto time_elapsed = std::chrono::duration_cast<std::chrono::miliseconds>(std::chrono::high_resolution_clock::now() - time_begin); \
	std::cout << "[PROFILE]: Function \"" << __FUNCTION__ << " took " << time_elapsed.count() << "s to execute" << std::endl;

#if XR_OS_WINDOWS && _DEBUG
#define TRACY_ENABLE
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

tracy::VkCtx* tracy_get_ctx();
void tracy_init_ctx(VkPhysicalDevice physical_device, VkDevice device, VkQueue queue, VkCommandBuffer cmd);

#define QUE_PROFILE ZoneScoped
#define QUE_PROFILE_FRAME FrameMark
#define QUE_PROFILE_SECTION(x) ZoneScopedN(x)
#define QUE_PROFILE_TAG(y, x) ZoneText(x, strlen(x))
#define QUE_PROFILE_LOG(text, size) TracyMessage(text, size)
#define QUE_PROFILE_VALUE(text, value) TracyPlot(text, value)
#define QUE_MARK_GPU_CONTEXT

#define QUE_INIT_GPU_PROFILER(physical_device, device, queue, cmd) tracy_init_ctx(physical_device, device, queue, cmd)
#define QUE_DESTROY_GPU_PROFILER TracyVkDestroy(tracy_get_ctx())

#define QUE_GPU_COLLECT(cmd) TracyVkCollect(tracy_get_ctx(), cmd);
#define QUE_GPU_ZONE(cmd, x) TracyVkZone(tracy_get_ctx(), cmd, x);

#else

#define QUE_PROFILE
#define QUE_PROFILE_FRAME
#define QUE_PROFILE_SECTION(x)
#define QUE_PROFILE_TAG(y, x)
#define QUE_PROFILE_LOG(text, size)
#define QUE_PROFILE_VALUE(text, value)
#define QUE_MARK_GPU_CONTEXT

#define QUE_INIT_GPU_PROFILER(physical_device, device, queue, cmd)
#define QUE_DESTROY_GPU_PROFILER

#define QUE_GPU_COLLECT(cmd)
#define QUE_GPU_ZONE(cmd, x)

#endif