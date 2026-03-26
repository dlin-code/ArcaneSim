#pragma once
#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN																		// GLFW_INCLUDE_VULKAN tells GLFW to include Vulkan headers for you.
#include <GLFW/glfw3.h>																			// Line 5-6 makes glfwCreateWindow Vulkan_aware.
#include <string>
#include <vector>
#include <map>
#include <optional>

#include <set>
#include <algorithm>	// Necessary for std::clamp

#include <cstdint>		// for uint16_t

#include "vertex.h"
#include "camera.h"
#include "particle.h"

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

struct UniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::vec3 lightPos;
	alignas(16) glm::vec3 viewPos;
};

struct PushConstants {
	glm::mat4 model;
};

struct InstanceData {
	glm::mat4 model;
};

class VulkanApplication
{
private:
	const uint32_t WIDTH = 800;																		// The current window size.
	const uint32_t HEIGHT = 600;

	const std::string SNOWMOUNTAIN_MODEL_PATH = "../../../models/great_mountain.obj";
	const std::string SNOWMOUNTAIN_TEXTURE_PATH = "../../../textures/mountain_diffuse.jpg";
	const std::string SNOWMOUNTAIN_NORMAL_MAP_PATH = "../../../textures/mountain_diffuse_normal.png";

	const std::string TOWER_MODEL_PATH = "../../../models/medieval_tower.obj";

	const std::string DEAD_TREE_MODEL_PATH = "../../../models/DeadTree_LoPoly.obj";
	const std::string DEAD_TREE_TEXTURE_PATH = "../../../textures/DeadTree_LoPoly_DeadTree_Diffuse.png";
	const std::string DEAD_TREE_NORMAL_MAP_PATH = "../../../textures/DeadTree_LoPoly_DeadTree_Normal.png";

	const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<VkImage> swapChainImages;

	/*const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 0.25f, 0.75f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.5f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 0.25f, 0.75f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.5f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {0.5f, 0.5f, 0.5f}, {1.0f, 1.0f}},
	};*/

	const std::vector<glm::vec3> skybox_vertices = {
		// Back
		{-1.0f,  1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{ 1.0f,	-1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{-1.0f,  1.0f, -1.0f},

		// Front
		{-1.0f, -1.0f, 1.0f},
		{-1.0f,  1.0f, 1.0f},
		{ 1.0f,  1.0f, 1.0f},
		{ 1.0f,  1.0f, 1.0f},
		{ 1.0f, -1.0f, 1.0f},
		{-1.0f, -1.0f, 1.0f},

		// Left
		{-1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},

		// Right
		{1.0f,  1.0f, -1.0f},
		{1.0f,  1.0f,  1.0f},
		{1.0f, -1.0f,  1.0f},
		{1.0f, -1.0f,  1.0f},
		{1.0f, -1.0f, -1.0f},
		{1.0f,  1.0f, -1.0f},

		// Top
		{-1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f, -1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{ 1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f,  1.0f},
		{-1.0f,  1.0f, -1.0f},

		// Bottom
		{-1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f,  1.0f},
		{ 1.0f, -1.0f, -1.0f},
		{-1.0f, -1.0f, -1.0f}
	};

	std::vector<Vertex> vertices;

	/*const std::vector<uint16_t> indices = {
		0, 1, 2 , 2, 3, 0,
		4, 5, 6 , 6, 7, 4
	};*/

	std::vector<uint32_t> indices;

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

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
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

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage snowMountainImage;
	VkDeviceMemory snowMountainImageMemory;

	VkImageView snowMountainImageView;
	VkSampler textureSampler;

	VkImage snowMountainNormalMapImage = VK_NULL_HANDLE;
	VkDeviceMemory snowMountainNormalMapImageMemory = VK_NULL_HANDLE;
	VkImageView snowMountainNormalMapImageView = VK_NULL_HANDLE;

	VkPhysicalDeviceProperties deviceProperties;

	// Skybox
	std::vector<Vertex> skyboxVertices;
	VkBuffer skyboxVertexBuffer;
	VkDeviceMemory skyboxVertexBufferMemory;

	VkImage skyboxImage;
	VkDeviceMemory skyboxImageMemory;
	VkImageView skyboxImageView;
	VkSampler skyboxSampler;

	VkDescriptorSetLayout skyboxDescriptorSetLayout;
	VkPipelineLayout skyboxPipelineLayout;
	VkPipeline skyboxPipeline;
	std::vector<VkDescriptorSet> skyboxDescriptorSets;

	// Medieval tower
	std::vector<Vertex> towerVertices;
	std::vector<uint32_t> towerIndices;

	VkBuffer towerVertexBuffer;
	VkDeviceMemory towerVertexBufferMemory;
	VkBuffer towerIndexBuffer;
	VkDeviceMemory towerIndexBufferMemory;

	// Dead tree
	std::vector<Vertex> deadTreeVertices;
	std::vector<uint32_t> deadTreeIndices;

	VkBuffer deadTreeVertexBuffer;
	VkDeviceMemory deadTreeVertexBufferMemory;
	VkBuffer deadTreeIndexBuffer;
	VkDeviceMemory deadTreeIndexBufferMemory;

	VkImage deadTreeImage;
	VkDeviceMemory deadTreeImageMemory;
	VkImageView deadTreeImageView;

	VkImage deadTreeNormalMapImage = VK_NULL_HANDLE;
	VkDeviceMemory deadTreeNormalMapImageMemory = VK_NULL_HANDLE;
	VkImageView deadTreeNormalMapImageView = VK_NULL_HANDLE;

	std::vector<VkDescriptorSet> deadTreeDescriptorSets;

	// Dead tree instancing
	std::vector<InstanceData> deadTreeInstances;
	VkBuffer deadTreeInstanceBuffer;
	VkDeviceMemory deadTreeInstanceBufferMemory;

	VkPipeline instancedPipeline;
	VkPipelineLayout instancedPipelineLayout;

	// Particles
	const int MAX_PARTICLES = 500;
	VkDescriptorSetLayout particleDescriptorSetLayout;
	VkPipelineLayout particlePipelineLayout;
	VkPipeline particlePipeline;

	std::vector<Particle> particles;
	std::vector<VkBuffer> particleVertexBuffers;
	std::vector<VkDeviceMemory> particleVertexBuffersMemory;
	std::vector<void*> particleVertexBuffersMapped;
	std::vector<VkDescriptorSet> particleDescriptorSets;

public:
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	Camera mainCamera;

	void initWindow();
	static void framebufferResizeCallback(GLFWwindow*, int, int);
	void createInstance();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();

	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void recordCommandBuffer(VkCommandBuffer, uint32_t);
	void createSyncObjects();
	void createVertexBuffer(const std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
	uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);

	void createBuffer(VkDeviceSize, VkBufferUsageFlags, VkMemoryPropertyFlags, VkBuffer&, VkDeviceMemory&);

	void copyBuffer(VkBuffer, VkBuffer, VkDeviceSize);

	void createIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
	void createUniformBuffers();

	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets, VkImageView textureImageView, VkImageView normalMapImageView, VkSampler textureSampler);

	void updateUniformBuffer(uint32_t);

	void createDepthResources();

	VkFormat findSupportedFormat(const std::vector<VkFormat>&, VkImageTiling, VkFormatFeatureFlags);
	VkFormat findDepthFormat();

	bool hasStencilComponent(VkFormat);

	void createImage(uint32_t, uint32_t, VkFormat, VkImageTiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage, VkImageViewType, VkFormat, VkImageAspectFlags, uint32_t);

	void createTextureImage(const std::string texturePath, VkImage& textureImage, VkDeviceMemory& textureImangeMemory);
	void createNormalMapImage(const std::string normalMapPath, VkImage& normalMapImage, VkDeviceMemory& normalMapImageMemory);

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer);

	void transitionImageLayout(VkImage, VkFormat, VkImageLayout, VkImageLayout, uint32_t);

	void copyBufferToImage(VkBuffer, VkImage, uint32_t, uint32_t, uint32_t);

	void createTextureImageView(VkImage textureImage, VkImageView& imageView);
	void createNormalMapImageView(VkImage normalMapImage, VkImageView& normalMapImageView);

	void createTextureSampler(VkSampler& sampler);

	void loadModel(const std::string& modelPath, std::vector<Vertex>& outVertices, std::vector<uint32_t>& outIndices);

	void initVulkan();

	void mainLoop();

	void drawFrame();

	void cleanupSwapChain();
	void recreateSwapChain();

	void createSkyboxVertexBuffer();
	void createSkyboxImage();
	void createSkyboxImageView();
	void createSkyboxSampler();
	void createSkyboxDescriptorSetLayout();
	void createSkyboxDescriptorSets();
	void createSkyboxPipeline();
	
	void createDeadTreeInstanceBuffer();
	void createInstancedPipeline();

	void initParticles();
	void updateParticles(float deltaTime);

	void createParticleVertexBuffers();
	void updateParticleVertexBuffer(uint32_t currentImage);

	void createParticlesPipeline();

	void createParticleDescriptorSetLayout();
	void createParticleDescriptorSets();

	void cleanUp();
};
