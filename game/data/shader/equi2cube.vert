#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout( push_constant ) uniform constants
{
	mat4 viewProj;
} PushConstants;

layout(location = 0) in vec3 a_Positions;
layout(location = 1) in vec3 a_Normals;
layout(location = 2) in vec3 a_Tangents;
layout(location = 3) in vec3 a_Bitangents;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 0) out vec2 o_TexCoord;
layout(location = 1) out vec3 o_Normal;
layout(location = 2) out vec3 o_Pos;

void main() {
    o_Pos = a_Positions;
    vec4 pos = PushConstants.viewProj * vec4(a_Positions, 1.0f);
    gl_Position = pos.xyww;
	o_TexCoord = a_TexCoord;
	o_Normal = a_Normals;
}