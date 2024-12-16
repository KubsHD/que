#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory.h>
#include <glm/glm.hpp>

enum GraphicsAPI_Type : uint8_t {
	UNKNOWN,
	D3D11,
	D3D12,
	OPENGL,
	OPENGL_ES,
	VULKAN
};

enum TextureType {
	TT_DIFFUSE = 0,
	TT_NORMAL = 1,
	TT_HDRI = 2
};

using String = std::string;

template<typename T>
using Vector = std::vector<T>;

using Vec2 = glm::vec2;

using Vec3 = glm::vec3;

using Vec4 = glm::vec4;

using Path = std::filesystem::path;

using Quat = glm::quat;

// filesystem

namespace fs = std::filesystem;

using Path = fs::path;

template<typename T>
using ref = std::shared_ptr<T>;