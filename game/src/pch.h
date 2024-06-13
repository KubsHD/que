#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext.hpp>
#include <glm/gtx/quaternion.hpp>

#include <entt/entt.hpp>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <filesystem>

#include <lib/netimgui/NetImgui_Api.h>
#include <lib/im3d/im3d.h>
#include <lib/tiny_gltf.h>
#include <lib/stb_image.h>

#include <functional>


#include <chrono>
#include <memory>
#include <functional>
#include <math.h>


#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

#include <iostream>
#include <memory>
#include <cstdarg>
#include <thread>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <map>

#include <lib/json.hpp>



// Platform headers
#if defined(_WIN32)
#define XR_USE_PLATFORM_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Unknwn.h>
#endif

#if defined(XR_OS_ANDROID)
#define XR_USE_PLATFORM_ANDROID
#define VK_USE_PLATFORM_ANDROID
#include <jni.h>
#endif

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <lib/vk_mem_alloc.h>
#include <lib/VkBootstrap.h>


#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <core/types.h>