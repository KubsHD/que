// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout( push_constant ) uniform constants
{
	mat4 model;
} PushConstants;

layout(location = 0) in vec3 a_Positions;
layout(location = 1) in vec3 a_Normals;
layout(location = 2) in vec3 a_Tangents;
layout(location = 3) in vec3 a_Bitangents;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 0) out vec3 o_Pos;

void main() {
    o_Pos = a_Positions;
    mat4 rotView = mat4(mat3(view)); // remove translation from the view matrix
    vec4 clipPos = proj * rotView * vec4(o_Pos, 1.0);

    vec4 pos = viewProj * PushConstants.model * vec4(a_Positions, 1.0f);

    gl_Position = pos.xyww;
}