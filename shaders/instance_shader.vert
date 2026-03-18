#version 450

layout(binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;
	vec3 lightPos;
	vec3 viewPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;
layout(location = 6) in vec4 instanceMatrix0;
layout(location = 7) in vec4 instanceMatrix1;
layout(location = 8) in vec4 instanceMatrix2;
layout(location = 9) in vec4 instanceMatrix3;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out mat3 fragTBN;

void main()	{
	mat4 instanceModel = mat4(instanceMatrix0, instanceMatrix1, instanceMatrix2, instanceMatrix3);
	
	vec4 worldPos = instanceModel * vec4(inPosition, 1.0);
	fragPos = worldPos.xyz;

	gl_Position = ubo.proj * ubo.view * worldPos;

	fragColor = inColor;
	fragTexCoord = inTexCoord;
	fragNormal = mat3(instanceModel) * inNormal;

	vec3 T = normalize(mat3(instanceModel) * inTangent);
	vec3 N = normalize(mat3(instanceModel) * inNormal);
	vec3 B = normalize(cross(N, T));
	fragTBN = mat3(T, B, N);
}