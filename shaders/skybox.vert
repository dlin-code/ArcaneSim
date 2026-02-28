#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 lightPos;
	vec3 viewPos;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragTexCoord;

void main() {
	mat4 viewNoTranslation = mat4(mat3(ubo.view));

	vec4 pos = ubo.proj * viewNoTranslation * vec4(inPosition, 1.0);

	gl_Position = pos.xyww;

	fragTexCoord = inPosition;
}