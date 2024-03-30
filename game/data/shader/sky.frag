#version 450

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in vec3 o_Pos;

layout(location = 0) out vec4 o_Color;


layout(set = 0, binding = 1) uniform sampler2D tex1;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = SampleSphericalMap(normalize(o_Pos)); // make sure to normalize localPos
	vec3 color = texture(tex1, uv).xyz;
	o_Color = vec4(color, 1.0f);
}