#version 450

layout(binding = 0) uniform uniformBufferObject {
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 lightPos;
	vec3 viewPos;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;
layout(binding = 2) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in mat3 fragTBN;

layout(location = 0) out vec4 outColor;

void main() {
	vec3 lightPos = ubo.lightPos;
	vec3 viewPos = ubo.viewPos;
	vec3 lightColor = vec3(1.3, 1.3, 1.3);

	vec3 objectColor = texture(texSampler, fragTexCoord).rgb;

	vec3 normalMapColor = texture(normalSampler, fragTexCoord).rgb;
	vec3 normal = normalize(normalMapColor * 2.0 - 1.0);
	normal = normalize(fragTBN * normal);

	// Ambient
	float ambientStrength = 0.35;
	vec3 ambient = ambientStrength * lightColor;

	// Diffuse
	vec3 norm = normalize(fragNormal);
	vec3 lightDir = normalize(lightPos - fragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// Specular
	float specularStrength = 0.6;
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor;

	outColor = vec4(result, 1.0);
}