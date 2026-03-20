#version 450

layout(binding = 0) uniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 lightPos;
	vec3 viewPos;
} ubo

layout(location = 0) in vec3 inPosition;

void main() {
	gl_Position = proj * view * vec4(inPosition, 1.0);
	gl_PointSize = 3.0;
}