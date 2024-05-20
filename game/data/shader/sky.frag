#version 450

layout(location = 0) in vec3 i_Pos;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 1) uniform samplerCube tex1;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    //vec2 uv = SampleSphericalMap(normalize(i_Pos)); // make sure to normalize localPos
	//vec3 color = texture(tex1, uv).xyz;

    vec3 envColor = texture(tex1, i_Pos).rgb;
    
    //envColor = envColor / (envColor + vec3(1.0));
    //envColor = pow(envColor, vec3(1.0/2.2)); 

	o_Color = vec4(envColor, 1.0f);
}