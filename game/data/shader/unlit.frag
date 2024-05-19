#version 450

#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in vec3 i_Pos;
layout(location = 3) in vec3 i_WorldPos;
layout(location = 4) in mat3 i_TBN;

layout(location = 0) out vec4 o_Color;

void main() {
	o_Color = vec4(vec3(0.2f, 0.3f, 0.4f), 1.0f);
}