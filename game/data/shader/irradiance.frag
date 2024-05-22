#version 450

layout(location = 0) in vec2 i_TexCoord;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in vec3 i_Pos;

layout(location = 0) out vec4 o_Color;

layout(set = 0, binding = 1) uniform samplerCube tex1;

const float PI = 3.14159265359;

void main() {

	vec3 normal = normalize(i_Pos);
	vec3 irradiance = vec3(0.0);

	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, normal));
	up         = normalize(cross(normal, right));

	float sampleDelta = 0.025;
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal; 

            irradiance += texture(tex1, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));

	o_Color = vec4(irradiance, 1.0f);
}