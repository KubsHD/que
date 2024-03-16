// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(set = 0,binding = 0) uniform SceneData {
	mat4 viewProj;
	mat4 view;
	mat4 proj;
};

layout( push_constant ) uniform constants
{
	mat4 model;
} PushConstants;

layout(location = 0) in vec3 a_Positions;
layout(location = 1) in vec3 a_Normals;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec2 o_TexCoord;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out vec3 o_Pos;

void main() {
    o_Pos = a_Positions;
    vec4 pos = viewProj * PushConstants.model * vec4(a_Positions, 1.0f);
    gl_Position = pos.xyww;
	o_TexCoord = a_TexCoord;
	o_Normal = a_Normals;
}