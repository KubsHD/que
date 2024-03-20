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

layout(set = 0, binding = 1) uniform samplerCube tex_sky;

layout(set = 1, binding = 0) uniform InstanceData {
	mat4 model;
	mat4 modelInvTrans;
};


layout(location = 0) in vec3 a_Positions;
layout(location = 1) in vec3 a_Normals;
layout(location = 2) in vec3 a_Tangents;
layout(location = 3) in vec3 a_Bitangents;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 0) out vec2 o_TexCoord;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out vec3 o_Pos;
layout(location = 3) out vec3 o_WorldPos;
layout(location = 4) out mat3 o_TBN;


void main() {
    o_Pos = a_Positions;
    gl_Position = viewProj * model * vec4(a_Positions, 1.0f);
	o_TexCoord = a_TexCoord;
	o_Normal = a_Normals;
	o_WorldPos = vec3(model * vec4(a_Positions, 1.0f));

	vec3 T = normalize(vec3(model * vec4(a_Tangents,   0.0)));
	vec3 B = normalize(vec3(model * vec4(a_Bitangents, 0.0)));
	vec3 N = normalize(vec3(model * vec4(a_Normals,    0.0)));
	o_TBN = mat3(T, B, N);
}