#pragma once
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN																		// GLFW_INCLUDE_VULKAN tells GLFW to include Vulkan headers for you.
#include <GLFW/glfw3.h>																			// Line 5-6 makes glfwCreateWindow Vulkan_aware.

#include <vector>
#include <map>
#include <optional>

#include <set>
#include <algorithm>	// Necessary for std::clamp

#include <cstdint>		// for uint16_t

#include "vertex.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Holds the indices of the queue families for graphics and presentation, if found. ".isComplete()" returns true if both are set.
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	};
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class VulkanApplication
{
private:
	const uint32_t WIDTH = 800;																		// The current window size.
	const uint32_t HEIGHT = 600;

	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<VkImage> swapChainImages;

	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 0.25f, 0.75f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.5f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}},
	};

	const std::vector<uint16_t> indices = {
		0, 1, 2 , 2, 3, 0
	};

	GLFWwindow* window = nullptr;

	VkInstance instance;
	VkSurfaceKHR surface;
	VkDevice device;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;								// Start with no device selected.
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
	
	bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	int rateDeviceSuitability(VkPhysicalDevice physicalDevice);
	
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities/*, GLFWwindow* window*/);

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;

	VkPipeline graphicsPipeline{};

	VkDebugUtilsMessengerEXT debugMessenger;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	uint32_t currentFrame = 0;

	bool framebufferResized = false;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

public:
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	void initWindow(/*GLFWwindow*& window*/);
	static void framebufferResizeCallback(GLFWwindow*, int, int);
	void createInstance();
	void createSurface(/*GLFWwindow* window*/);
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain(/*GLFWwindow* window*/);
	void createImageViews();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void recordCommandBuffer(VkCommandBuffer, uint32_t);
	void createSyncObjects();
	void createVertexBuffer();
	uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);

	void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);

	void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);

	void createIndexBuffer();

	void initVulkan();

	void mainLoop(/*GLFWwindow* window*/);

	void drawFrame();

	void cleanupSwapChain();
	void recreateSwapChain();

	void cleanUp(/*GLFWwindow* window*/);
};
