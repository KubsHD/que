// Copyright 2023, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#version 450
layout(location = 0) in flat uvec2 i_TexCoord;

layout(location = 0) out vec4 o_Color;

void main() {
    uint i = i_TexCoord.x;
    //float light = 0.1 + 0.9 * clamp(i_Normal.g, 0.0, 1.0);
    o_Color = vec4(0.5);
}