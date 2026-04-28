#version 450

struct Particle {
	vec2 position;
	vec2 veocity;
	vec4 color;
}

layout(binding = 0) uniform uniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 lightPos;
	vec3 viewPos;
} ubo;

layout(std140, binding = 1) readonly buffer ParticleSSBOIn {
	Particle particlesIn[ ];
}

layout(std140, binding = 2) buffer ParticleSSBOOut {
	Particle particlesOut[ ];
}

layout(location = 0) in vec3 inPosition;

void main() {
	particlesOut[index].position = particlesIn[index].position + particlesIn[index].velocity.xy * ubo.deltaTime;
	gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
	gl_PointSize = 6.0;
}