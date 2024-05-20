#version 450

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in vec3 o_Pos;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 1) uniform samplerCube tex1;

void main() {
	o_Color = vec4(vec3(0.1f, 0.1f, 0.1f), 1.0f);
}