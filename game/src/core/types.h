#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <filesystem>

#include <vulkan/vulkan.h>

using String = std::string;

template<typename T>
using Vector = std::vector<T>;

using Vec2 = glm::vec2;

using Path = std::filesystem::path;