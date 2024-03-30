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