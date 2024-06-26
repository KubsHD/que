#pragma once

#include <chrono>


#define TIME_MEASURE_START						\
	auto time_begin = std::chrono::high_resolution_clock::now();		\


#define TIME_MEASURE_END																												\
	auto time_elapsed = std::chrono::duration_cast<std::chrono::miliseconds>(std::chrono::high_resolution_clock::now() - time_begin);	\
	std::cout << "[PROFILE]: Function \"" << __FUNCTION__ << " took " << time_elapsed.count() << "s to execute" << std::endl;			\



#if _WIN32 && _DEBUG
#define TRACY_ENABLE
#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

#define QUE_PROFILE ZoneScoped
#define QUE_PROFILE_FRAME(x) FrameMark
#define QUE_PROFILE_SECTION(x) ZoneScopedN(x)
#define QUE_PROFILE_TAG(y, x) ZoneText(x, strlen(x))
#define QUE_PROFILE_LOG(text, size) TracyMessage(text, size)
#define QUE_PROFILE_VALUE(text, value) TracyPlot(text, value)
#define QUE_MARK_GPU_CONTEXT 
#define QUE_GPU_COLLECT 
#else
#define QUE_PROFILE 
#define QUE_PROFILE_FRAME(x) 
#define QUE_PROFILE_SECTION(x) 
#define QUE_PROFILE_TAG(y, x)
#define QUE_PROFILE_LOG(text, size)
#define QUE_PROFILE_VALUE(text, value)
#define QUE_MARK_GPU_CONTEXT
#define QUE_GPU_COLLECT
#endif