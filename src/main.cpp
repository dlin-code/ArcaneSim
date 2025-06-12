#include <iostream>																				// iostream: For printing messages.
#include <stdexcept>																			// stdexcept: For throwing and catching errors.
#include <cstdlib>																				// cstdlib: For EXIT_SUCCESS / EXIT_FAILURE codes.

#define GLFW_INCLUDE_VULKAN																		// GLFW_INCLUDE_VULKAN tells GLFW to include Vulkan headers for you.
#include <GLFW/glfw3.h>																			// Line 5-6 makes glfwCreateWindow Vulkan_aware.

#include <vector>
#include <map>
#include <optional>

#include <set>

const uint32_t WIDTH = 800;																		// The current window size.
const uint32_t HEIGHT = 600;

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

void initWindow(GLFWwindow*& window) {															
	glfwInit();																					// Initialises GLFW.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);												// Disables OpenGL context (because we're using Vulkan).
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);													// Makes the window non-resizable (simplifies surface handling for now)

	window = glfwCreateWindow(WIDTH, HEIGHT, "ArcaneSim", nullptr, nullptr);					// Returns a pointer to the GLFW window.
}

VkInstance createVulkanInstance() {																
	// Sets up application + instance creation
	VkInstance instance;

	// Line 25-31: VkApplicationInfo
	// Describes your app to Vulkan. Mostly optional, but useful for drivers/tools.
	VkApplicationInfo appInfo{};															// Creates an empty VKApplicationInfo struct, initializing all fields to zero or null by default. It's a C-style struct used to describe your application to the Vulkan implementation/driver.
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;										// Every Vulkan struct starts with an sType field so the driver knows what kind of struct it's receiving. "VK_STRUCTURE_TYPE_APPLICATION_INFO" is a Vulkan constant telling the driver this is app info.
	appInfo.pApplicationName = "ArcaneSim";													// Pointer to a C-style string with your app's name. Some drivers or too may use this to display your app name in diagnostics, logs, or overlays.
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);									// Application version, using the "VK_MAKE_VERSION(major, minor, patch)" macro. Not strictly used by Vulkan itself, but some tools may use it for reporting, debbuging, or compatibility purposes.
	appInfo.pEngineName = "No Engine";														// Name of the graphics/game engine. Like app name, some tools/drivers use this for logging or statistics.
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);										// Version of your engine. Handy for tracking if later evolves the engine.
	appInfo.apiVersion = VK_API_VERSION_1_0;												// Specifies the minimum Vulkan API version your app requires. For maximum compatibility, starts with VK_API_VERSION_1_0 (the original release). Would need to set the appropriate constant if wanting to use features from newer versions (like 1.1, 1.2, 1.3). The driver will return an error if the required API version isn't supported.

	// Line 35-44: VkInstanceCreateInfo
	// Combines app info with extensions & layers.
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	// Line 47: Creates the Vulkan instance - the root of all Vulkan objects.
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance!");
	}

	return instance;
}

// It allows Vulkan to present rendered image to the window that's created.
void createVulkanSurface(VkInstance instance, GLFWwindow* window, VkSurfaceKHR *surface) {
	if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

// Holds the indices of the queue families for graphics and presentation, if found. ".isComplete()" returns true if both are set.
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	};
};


// Loop through all queue families for a given device.
// Records the first family that supports graphics and the first that supports presenting to the surface.
QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Look for suitable queue families
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// Check for graphics capability
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		// Check for present support
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	// Assign index to queue families that could be found
	return indices;
}


// Checks if the device supports the necessary queues for graphics and presentation.
bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices = findQueueFamilies(device, surface);

	return indices.isComplete();
}

int rateDeviceSuitability(VkPhysicalDevice device, VkSurfaceKHR surface) {
	// Query the device's general properties (name, type, limits, etc.)
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;	// Start with a zero score.

	// Favour discrete GPUs (like Nvdia or AMD cards) with a larger bonus.
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Add points based on the largest possible 2D image dimension (bigger is generally better for rendering).
	score += deviceProperties.limits.maxImageDimension2D;

	// Require support for geometry shaders.
	// If not supported, this device is disqualified (score = 0).
	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	if (!isDeviceSuitable(device, surface)) {
		return 0;
	}

	// Return the final score for this device.
	return score;
}

void pickVulkanPhysicalDevice(VkInstance& instance, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface) {
	// Start with no device selected.
	physicalDevice = VK_NULL_HANDLE;

	// Created a multimap to rank GPUs by score (higher is better).
	std::multimap<int, VkPhysicalDevice> candidates;

	// In here it asks Vulkan how many GPUs (physical devices) are available, writing the count to deviceCount.
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// Allocates a vector to hold all device handles, and calls vkEnumeratePhysicalDevices again (now with an array to write into).
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Score each device and add it to the candidates list.
	for (const auto& device : devices) {
		int score = rateDeviceSuitability(device, surface);

		candidates.insert(std::make_pair(score, device));
	}

	if (candidates.rbegin()->first > 0) {
		physicalDevice = candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}


void createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkDevice& device, VkQueue& graphicsQueue, VkQueue& presentQueue) {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	//queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = /*&queueCreateInfo*/queueCreateInfos.data();
	createInfo.queueCreateInfoCount = /*1*/static_cast<uint32_t>(queueCreateInfos.size());

	createInfo.pEnabledFeatures = &deviceFeatures;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	createInfo.enabledExtensionCount = /*0*/static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

int main() {
	GLFWwindow* window;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	try {
		initWindow(window);																		// Creates the window.
		VkInstance instance = createVulkanInstance();											// Creates Vulkan instance.

		createVulkanSurface(instance, window, &surface);
		pickVulkanPhysicalDevice(instance, physicalDevice, surface);

		createLogicalDevice(physicalDevice, surface, device, graphicsQueue, presentQueue);

		std::cout << "Vulkan instance created successfully!\n";  

		while (!glfwWindowShouldClose(window)) {												// Runs a basic event loop.
			glfwPollEvents();
		}

		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();

		return EXIT_SUCCESS;
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}