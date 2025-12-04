#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>

struct Vertex
{
	//glm::vec3 positions;
	glm::vec3 pos;
	glm::vec3 color;
	//glm::vec3 normal;
	//glm::vec2 uv;

	// Binding descriptions. Tells Vulkan how to pass vertex vector format to the vertex shader once it's been uploaded into GPU memory.
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription{};									// A vertex binding descibes at which rate to load data from memory throughout the vertices. It specifies the number of bytes between data entries and whether to move to the next data entry after each vertex or after each instance.
		bindingDescription.binding = 0;															// The binding parameter specifies the index of the binding in the array of bindings.
		bindingDescription.stride = sizeof(Vertex);												// The stride parameter specifies the number of bytes from one entry to the next.
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;								// In this parameter I choose to move the next data entry after each vertex.

		return bindingDescription;
	}

	// Attribute descriptions
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

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

		return attributeDescriptions;
	}
};