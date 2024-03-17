// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#version 450
#extension GL_KHR_vulkan_glsl : enable


layout(set = 0,binding = 0) uniform SceneData {
	mat4 viewProj;
	mat4 view;
	mat4 proj;
	vec3 camPos;
};

layout(set = 1, binding = 0) uniform InstanceData {
	mat4 model;
	mat4 modelInvTrans;
};

layout(location = 0) in vec3 a_Positions;
layout(location = 1) in vec3 a_Normals;
layout(location = 2) in vec2 a_TexCoord;

layout(location = 0) out vec2 o_TexCoord;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out vec3 o_Pos;
layout(location = 3) out vec3 o_WorldPos;


void main() {
    o_Pos = a_Positions;
    gl_Position = viewProj * model * vec4(a_Positions, 1.0f);
	o_TexCoord = a_TexCoord;
	o_Normal = a_Normals;
	o_WorldPos = vec3(model * vec4(a_Positions, 1.0f));
}