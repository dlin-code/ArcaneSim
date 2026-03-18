#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	// Binding descriptions. Tells Vulkan how to pass vertex vector format to the vertex shader once it's been uploaded into GPU memory.
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};									// A vertex binding descibes at which rate to load data from memory throughout the vertices. It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
		bindingDescription.binding = 0;															// The binding parameter specifies the index of the binding in the array of bindings.
		bindingDescription.stride = sizeof(Vertex);												// The stride parameter specifies the number of bytes from one entry to the next.
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;								// In this parameter I choose to move the next data entry after each vertex.

		return bindingDescription;
	}

	// Attribute descriptions
	static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};

		// Position
		attributeDescriptions[0].binding = 0;													// It tells Vulkan from which binding the per-vertex data comes.
		attributeDescriptions[0].location = 0;													// It references the "location" directive of the input in the vertex shader.
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;								// It describes the type of data for the attribute, the format are specified using the same enumeration as color formats.
		attributeDescriptions[0].offset = offsetof(Vertex, pos);								// It specifies the number of bytes since the start of the per-vertex data to read from. 
		
		// Color
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		// Texture coordinate
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		// Normal
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		// Tangent
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Vertex, tangent);

		// Bitangent
		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(Vertex, bitangent);

		return attributeDescriptions;
	}

	static VkVertexInputBindingDescription getInstanceBindingDescription() {
		VkVertexInputBindingDescription instanceBindingDescription{};									// A vertex binding descibes at which rate to load data from memory throughout the vertices. It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
		instanceBindingDescription.binding = 1;															// The binding parameter specifies the index of the binding in the array of bindings.
		instanceBindingDescription.stride = sizeof(glm::mat4);												// The stride parameter specifies the number of bytes from one entry to the next.
		instanceBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;								// In this parameter I choose to move the next data entry after each vertex.

		return instanceBindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 4> getInstanceAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 4> instanceAttributeDescriptions{};

		instanceAttributeDescriptions[0].binding = 1;
		instanceAttributeDescriptions[0].location = 6;
		instanceAttributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		instanceAttributeDescriptions[0].offset = 0;

		instanceAttributeDescriptions[1].binding = 1;
		instanceAttributeDescriptions[1].location = 7;
		instanceAttributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		instanceAttributeDescriptions[1].offset = sizeof(glm::vec4);

		instanceAttributeDescriptions[2].binding = 1;
		instanceAttributeDescriptions[2].location = 8;
		instanceAttributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		instanceAttributeDescriptions[2].offset = sizeof(glm::vec4) * 2;

		instanceAttributeDescriptions[3].binding = 1;
		instanceAttributeDescriptions[3].location = 9;
		instanceAttributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		instanceAttributeDescriptions[3].offset = sizeof(glm::vec4) * 3;

		return instanceAttributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color 
				&& texCoord == other.texCoord && normal == other.normal
				&& tangent == other.tangent && bitangent == other.bitangent;
	}
};

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1) ^
				(hash<glm::vec3>()(vertex.normal) << 1) ^
				(hash<glm::vec3>()(vertex.tangent) << 1) ^
				(hash<glm::vec3>()(vertex.bitangent) << 1);
		}
	};
}