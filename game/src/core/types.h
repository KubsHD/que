#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <glm/glm.hpp>

enum GraphicsAPI_Type : uint8_t {
	UNKNOWN,
	D3D11,
	D3D12,
	OPENGL,
	OPENGL_ES,
	VULKAN
};

using String = std::string;

template<typename T>
using Vector = std::vector<T>;

using Vec2 = glm::vec2;

using Vec3 = glm::vec3;

using Path = std::filesystem::path;

using Quat = glm::quat;

// filesystem

namespace fs = std::filesystem;

using Path = fs::path;