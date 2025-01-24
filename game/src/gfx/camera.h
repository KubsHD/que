#pragma once

#include <core/types.h>

struct Camera
{
    Vec3 position;
    glm::quat rotation;
    Vec3 rotation_euler;
};

struct CameraRenderData
{
    Vec3 position;

    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
};