// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#version 450
layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in vec3 o_Pos;

layout(location = 0) out vec4 o_Color;
layout(binding = 1) uniform sampler2D tex1;

void main() {
	vec3 color = texture(tex1,i_TexCoord).xyz;
	o_Color = vec4(color, 1.0f);
}