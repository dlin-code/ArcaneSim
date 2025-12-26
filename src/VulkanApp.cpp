#include "VulkanApp.h"
#include <stdexcept>
#include <iostream>
#include <fstream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

void VulkanApplication::initWindow(/*GLFWwindow*& windowPtr*/) {
	if (!glfwInit()) {																			// Initialises GLFW.
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);												// Disables OpenGL context (because we're using Vulkan).
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);													// Makes the window non-resizable (simplifies surface handling for now)

	window = glfwCreateWindow(WIDTH, HEIGHT, "ArcaneSim", nullptr, nullptr);					// Returns a pointer to the GLFW window.
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

	if (!window) {
		throw std::runtime_error("Failed to create GLFW window");
	}
}

void VulkanApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<VulkanApplication*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void VulkanApplication::createInstance() {

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

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	debugCreateInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void*) -> VkBool32 {
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
			return VK_FALSE;
	};
	debugCreateInfo.pUserData = nullptr;

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;

		createInfo.pNext = nullptr;
	}

	std::cout << "Enabled Extensions:\n";
	for (uint32_t i = 0; i < createInfo.enabledExtensionCount; ++i) {
		std::cout << "\t" << createInfo.ppEnabledExtensionNames[i] << "\n";
	}

	// Creates the Vulkan instance - the root of all Vulkan objects.
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance!");
	}

	if (enableValidationLayers) { 
		if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create debug messenger!");
		}
	}
}

// It allows Vulkan to present rendered image to the window that's created.
void VulkanApplication::createSurface(/*GLFWwindow* window*/) {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

// Loop through all queue families for a given device.
// Records the first family that supports graphics and the first that supports presenting to the surface.
QueueFamilyIndices VulkanApplication::findQueueFamilies(VkPhysicalDevice physicalDevice) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	// Look for suitable queue families
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// Check for graphics capability
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		// Check for present support
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
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

bool VulkanApplication::checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

SwapChainSupportDetails VulkanApplication::querySwapChainSupport(VkPhysicalDevice physicalDevice) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

// Checks if the device supports the necessary queues for graphics and presentation.
bool VulkanApplication::isDeviceSuitable(VkPhysicalDevice physicalDevice) {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

int VulkanApplication::rateDeviceSuitability(VkPhysicalDevice physicalDevice) {
	// Query the device's general properties (name, type, limits, etc.)
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

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

	if (!isDeviceSuitable(physicalDevice)) {
		return 0;
	}

	// Return the final score for this device.
	return score;
}

void VulkanApplication::pickPhysicalDevice() {

	// Created a multimap to rank GPUs by score (higher is better).
	std::multimap<int, VkPhysicalDevice> candidates;

	// In here it asks Vulkan how many GPUs (physical devices) are available, writing the count to deviceCount.
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// Allocates a vector to hold all device handles, and calls vkEnumeratePhysicalDevices again (now with an array to write into).
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

	// Score each device and add it to the candidates list.
	for (const auto& candidateDevice : physicalDevices) {
		int score = rateDeviceSuitability(candidateDevice);

		candidates.insert(std::make_pair(score, candidateDevice));
	}

	if (candidates.rbegin()->first > 0) {
		physicalDevice = candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void VulkanApplication::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

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

	/*const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};*/

	createInfo.enabledExtensionCount = /*0*/static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

VkSurfaceFormatKHR VulkanApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

VkPresentModeKHR VulkanApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities/*, GLFWwindow* window*/) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VulkanApplication::createSwapChain(/*GLFWwindow* window*/) {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);

	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities/*, window*/);

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
}

void VulkanApplication::createImageViews() {
	// Resize the list to fit all of the image views that's gonna be created
	swapChainImageViews.resize(swapChainImages.size());

	// Iterates over all of the swap chain images.
	for (size_t i = 0; i < swapChainImages.size(); i++) {
		//VkImageViewCreateInfo createInfo{};
		//createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		//createInfo.image = swapChainImages[i];

		//// The "viewType" and "format" fields specify how the image data should be interpreted.
		//createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;		// The "viewType" parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
		//createInfo.format = swapChainImageFormat;

		//// The "components" field allows you to swizzle the color channels around. In here it sticks to the default mapping.
		//createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		//createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		//createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		//createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// The "subresourceRange" field describes what the image's purpose is and which part of the image should be accessed.
		// In here the images will be used as color targets without any mipmapping levels or multiple layers.
		//createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		//createInfo.subresourceRange.baseMipLevel = 0;
		//createInfo.subresourceRange.levelCount = 1;
		//createInfo.subresourceRange.baseArrayLayer = 0;
		//createInfo.subresourceRange.layerCount = 1;

		//// Creating the image view
		//if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
		//	throw std::runtime_error("failed to create image visual!");
		//}

		swapChainImageViews[i] = createImageView(
			swapChainImages[i],
			swapChainImageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}
}

// Reads the contents of a spv file, which is a raw binary data, into a std::vector<char>
// "static" is been used here as it prevents this function pollute the global symbol space.
//It's apart of "VulkanApp.cpp"'s private logic, not something to expose to other files.
//If other files also define a readFile(), you won't get linker conflicts.
static std::vector<char> readFile(const std::string& filename) {

	// Open at end and read the file as binary file (avoid text transformations) to get file size right away
	// Creates an input file stream to read the file named "filename".
	// "std::ios::binary": Open the file in binary code, to prevent text mode line-ending conversions (very important for .spv).
	// "std::ios::ate": Start reading at the end of the file, which allows us to immediately use file.tellg() to get the file size.
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file: " + filename);
	}
	
	// "file.tellg()" tell us the current position in the file - since we opened it at the end, this equals the file's total size in bytes.
	size_t fileSize = (size_t)file.tellg();

	if (fileSize == 0) {
		throw std::runtime_error("Shader file is empty: " + filename);
	}
	if (fileSize % 4 != 0) {
		std::cout << "File size modulo 4: " << (fileSize % 4) << std::endl;
		throw std::runtime_error("Invalid SPIR_V file size (not divisible by 4): " + filename);
	}

	std::vector<char> buffer(fileSize);

	// Moves the "get" position back to the start of the file.
	file.seekg(0);

	// Reads "fileSize" bytes from the file into the buffer.
	// "buffer.data()" points to buffer[0].
	// "fileSize" is the maximum iteration count for the buffer, like buffer.size();
	file.read(buffer.data(), fileSize);

	// Closes the file to release the resource.
	file.close();

	// Return the entire buffer to the caller - now holding the full contents of the shader .spv file.
	return buffer;
}

// This function will take a buffer with the bytecode as parameter and create a VkShaderModule from it.
VkShaderModule VulkanApplication::createShaderModule(const std::vector<char>& code) {
	// This creates and zero-initializes a struct that will describe how to create the shader module.
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	// This tells Vulkan how many bytes are in the shader code.
	createInfo.codeSize = code.size()/* * sizeof(uint32_t)*/;
	// Because "std::vector<char>" is a byte array (char*), and Vulkan expects shader code as an array of 32_bit word (uint32_t*).
	// So "reinterpret_cast" is being used here so Vulkan can treat the byte array as an 32-bit word array.
	// This works because SPIR-V is defined in 4-byte words, and Vulkan's spec requires that "codeSize" must be a multiple of 4; and "pCode" must point to properly aligned 32-bit data.
	// The compiler ensures alignment when using "std::vector<char>".
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	// Calls the actual Vulkan function to create a shader module..
	// Fills in "shaderModule" with the GPU-side handle.
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void VulkanApplication::createRenderPass() {
	// Attachment description
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;												// Format must match thr swapchain image (e.g., VK_FORMAT_R8G8B8A8_SRGB)
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;											// No multisampling (MSAA disabled)

	// The "loadOP" and "storeOP" determine what to do with the data in the attachment before rendering and after rendering
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;										// Clear framebuffer to a color at start of render pass
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;										// Store the rendered contents (we want to keep the result for presentation)

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;							// We don't use stencil, so it can be ignored
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;							// Since we don't use stencil, so there's no need to store stencil results

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;									// We don't care about previous contents
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;								// Transition to present layout so it can be shown on screen

	// Subpasses and attachment references
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;															// Index of the attachment in the render pass (only one, so 0)
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;						// OPtimal layout for color output during rendering

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;								// This subpass is for the graphics pipeline
	subpass.colorAttachmentCount = 1;															// One color attachment will be used
	subpass.pColorAttachments = &colorAttachmentRef;											// Pointer to our color attachment reference.
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask =
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = 
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	// Render pass
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());															// One attachment (color)
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;															// One subpass
	renderPassInfo.pSubpasses = &subpass;

	/*VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;*/

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanApplication::createGraphicsPipeline() {
	// Loads SPIR-V shader binaries into memory using "readFile" utility.
	// "vertShaderCode" and "fragShaderCode" contain raw shade bytecode (in std::vector<char>).
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	std::cout << "Program read - vertShaderCode size: " << vertShaderCode.size() << std::endl;
	std::cout << "Program read - fragShaderCode size: " << fragShaderCode.size() << std::endl;

	if (vertShaderCode.size() == 0) {
		std::cerr << "Vertex shader file is empty!" << std::endl;
	}

	if (fragShaderCode.size() == 0) {
		std::cerr << "Fragment shader file is empty!" << std::endl;
	}

	std::cout << "Creating vertex shader module...\n";
	vertShaderModule = createShaderModule(vertShaderCode);
	std::cout << "Vertex shader module created.\n";

	std::cout << "Creating fragment shader module...\n";
	fragShaderModule = createShaderModule(fragShaderCode);
	std::cout << "Fragment shader module created.\n";

	// Creates Vulkan shader module using the SPIR-V code by passing the binary into "vkCreateShaderModule" via helper function.
	/*VkShaderModule *///vertShaderModule = createShaderModule(vertShaderCode);
	/*VkShaderModule *///fragShaderModule = createShaderModule(fragShaderCode);

	// Creating shader stage info structs. These structs describes which shaders to use and how.
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;			// Tells Vulkan what kind of struct this is.
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;										// Specifies this is the vertex shader stage.
	vertShaderStageInfo.module = vertShaderModule;												// The Vulkan shader module you created.
	vertShaderStageInfo.pName = "main";															// The entry point function name in your shader.

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;									// Specifies this is a fragment shader stage.
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	// Vulkan expects both shader stages to be passed together into the pipeline as an array.
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Tell Vulkan we want to dynamically specify viewport and scissor rectangle at draw time.
	// These values will NOT be hardcoded into the pipeline; we'll set them later with vkCmdSetViewport/scissor.
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,																// Viewport (position + size) will be dynamic.
		VK_DYNAMIC_STATE_SCISSOR																// Scissor (clipping region) will also be dynamic.
	};

	// This struct describes the dynamic state feature to the pipeline creation.
	// We pass in our list of states (above), and Vulkan marks them as "externally provided later".
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;					// Must be set to this exact enum.
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());				// How many dynamic states we're using.
	dynamicState.pDynamicStates = dynamicStates.data();											// Pointer to the first element of our list.

	// Vertex input
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();
	
	// This structure describes the format of the vertex data that will be passed to the vertex shader.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Input assembly
	// This struct tells Vulkan how to interpret your list of vertices.
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;			// Structure type. It's required in Vulkan so the driver knows what's been passing.
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;								// This tells Vulkan how to connect the vertices. "VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST" means: Every three vertices form a triangle; and vertices do not share between triangles
	inputAssembly.primitiveRestartEnable = VK_FALSE;											// Relevant for strip topologies like "TRIANGLE_STRIP" or "LINE_STRIP". If set to "VK_TRUE", you can insert a special index (like '0xFFFF') in an index buffer to restart the strip. But since "TRIANGLE_LIST" is the one been used in this case, so this is set to "VK_FALSE".
	
	// Viewports and scissors
	VkViewport viewport{};																		// "VkViewport" defines the transformation from normalized device coordinates (NDC) -> window coordinates.
	viewport.x = 0.0f;																			// "x" and "y" represents the top left corner of the viewport in pixels.
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;												// "width" and "height" are the size of the viewport in pixels. Usually set to swap chain size.
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;																	// The "minDepth" and "maxDepth" values specify the range of depth values to use for the framebuffer.
	viewport.maxDepth = 1.0f;

	// The scissor defines what region of the screen to actually allow drawing in. Pixels outside the scissor rectangle will be discarded - no fragment shader will run for them.
	VkRect2D scissor{};
	scissor.offset = {0, 0};																	// Top-left corner of the scissor rectangle.
	scissor.extent = swapChainExtent;															// Width and height of the scissor area.														

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;															// Currently have one viewport.
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;																// Currently have one scissor rectangle.
	viewportState.pScissors = &scissor;

	// Rasterizer
	// It takes the geometry that is shaped by the vertices by the vertices from the vertex shader and turns it into fragments to be colored by the fragment shader.
	// It also performs depth testing, face culling and the scissor test, and it can be configured to output fragments that fill entire polygons or just just the edges (wireframe rendering).
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;														// If it's set to "VK_TRUE", then fragments that are beyond the near and far planes are clamped to them as opposed to discarding them.

	rasterizer.rasterizerDiscardEnable = VK_FALSE;												// If it's set to "VK_TRUE", then geometry never passes through the rasterizer stage, which disables any output to the framebuffer.

	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;												// It determines how fragments are generated for geometry. In this case "VK_POLYGON_MODE_FILL" fill the area of the polygon with fragments.

	rasterizer.lineWidth = 1.0f;																// It descries the thickness of lines in terms of number of fragments. 

	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;												// It determines the type of face culling to use, in this case it removes triangles facing away.
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;												// It specifies the vertex order for faces to be considered front-facing and can be clockwise or counterclockwise, in this case it's front face.

	rasterizer.depthBiasEnable = VK_FALSE;														// This affects how fragments are shifted along the Z-axis. Mostly used in shadow mapping to avoid z-fighting.
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};																								// This struct configures multisampling, which is one of the way to perform anti-alising. It works by combining the fragment shader results of multiple polygons that rasterize to the same pixel.
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;																										// Disables per-fragment sample shading. When "VK_TRUE", fragments are shaded multiple times (more accurate but slower).
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;																							// Specifies how many samples per pixel.
	multisampling.minSampleShading = 1.0f;																												// If "sampleShadingEnable" were on, this would specify minimum shading rate. Where in this case "1.0f" would fully shade every sample (highest quality).
	multisampling.pSampleMask = nullptr;																												// Sample masks allow per-sample control of multisampling. In this case "nullptr" means using the default behaviour.
	multisampling.alphaToCoverageEnable = VK_FALSE;																										// If enabled, uses alpha value to affect coverage (used for transparency and MSAA/Multisample Anti-Aliasing). It should be leave off unless blend transparent objects with MSAA is required later.
	multisampling.alphaToOneEnable = VK_FALSE;																											// If enabled, force alpha to 1.0 for all MSAA samples (rarely used); also mainly relevant for advanced transparency techniques.

	// Depth and stencil buffer
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.minDepthBounds = 0.0f;
	depthStencilState.maxDepthBounds = 1.0f;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = {};
	depthStencilState.back = {};

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};																							// This struct contains the configuration (a struct for controlling how blending is applied) per attached framebuffer.
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;	// This sets which components (Red, Green, Blue, Alpha) are written to the framebuffer. Here all four of them has enabled.
	colorBlendAttachment.blendEnable = VK_FALSE;																										// If set to "VK_FALSE", Vulkan skips blending and write the fragment shader's output color directly to the framebuffer color. If "VK_TRUE", Vulkan will blend the new fragment color with the existing framebuffer color using the setting (srcColorBlendFactor, dstColorBlendFactor, etc).
	
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;																						// These control RGB blending using standard alpha blending.
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;																					// Final RGB = srcAlpha * newColor + (1 - srcAlpha) * oldColor -> framebuffer gets fully overwritten with new fragment color.
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;																						// This affects how the alpha channel (transparency) is written. This setup keeps the new alpha unchanged (fully replaces it).
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};																								// This struct contains the global color blending settings used acorss all attachments.
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	
	colorBlending.logicOpEnable = VK_FALSE;																												// In here the logical operations (like XOR or AND) are not been used to combine colors as it's disabled. In here I'm using standard blending instead.
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	
	colorBlending.attachmentCount = 1;																													// In here we're telling Vulkan that there's one framebuffer attachment, and passing in the config for it. 
	colorBlending.pAttachments = &colorBlendAttachment;
	
	colorBlending.blendConstants[0] = 0.0f;																												// Only used if blend factors are set to VK_BLEND_FACTOR_CONSTANT_COLOR, which is not the case here.
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;																			// Struct type for creating a pipeline layout.
	pipelineLayoutInfo.setLayoutCount = 1;																												// No descriptor sets used for now. A descriptor set layout defines what kind of GPU resources (like textures, uniform buffers, etc.) the shaders can access and how those resources are bound.
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;																											// Pointer to descriptor set layouts (unused)
	pipelineLayoutInfo.pushConstantRangeCount = 0;																										// No push constants used for now. A push constant buffer is a small chunk of data (a few bytes) that you can quickly update and pass to shaders - much faster than updating a buffer.
	pipelineLayoutInfo.pPushConstantRanges = nullptr;																									// Pointer to push constant ranges (unused)

	// Create the pipeline layout object using the above info
	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	std::cout << "stageCount: " << pipelineInfo.stageCount << std::endl;
	std::cout << "pStages is null? " << (pipelineInfo.pStages == nullptr ? "yes" : "no") << std::endl;
	std::cout << "pVertexInputState is null? " << (pipelineInfo.pVertexInputState == nullptr ? "yes" : "no") << std::endl;
	std::cout << "pInputAssemblyState is null? " << (pipelineInfo.pInputAssemblyState == nullptr ? "yes" : "no") << std::endl;
	std::cout << "pViewportState is null? " << (pipelineInfo.pViewportState == nullptr ? "yes" : "no") << std::endl;
	std::cout << "pRasterizationState is null? " << (pipelineInfo.pRasterizationState == nullptr ? "yes" : "no") << std::endl;
	std::cout << "pMultisampleState is null? " << (pipelineInfo.pMultisampleState == nullptr ? "yes" : "no") << std::endl;
	std::cout << "pColorBlendState is null? " << (pipelineInfo.pColorBlendState == nullptr ? "yes" : "no") << std::endl;
	std::cout << "pDynamicState is null? " << (pipelineInfo.pDynamicState == nullptr ? "yes" : "no") << std::endl;
	std::cout << "layout: " << pipelineInfo.layout << std::endl;
	std::cout << "renderPass: " << pipelineInfo.renderPass << std::endl;

	VkDebugUtilsMessengerCallbackDataEXT* callbackData = nullptr;

	VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);

	/*if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}*/

	if (result != VK_SUCCESS) {
		std::cerr << "vKCreateGraphicsPipelines failed! Error code: " << result << std::endl;
		throw std::runtime_error("failed to create graphics pipeline!");
	}
}

void VulkanApplication::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainImageViews.size());																							// Resizing the container to hold all of the framebuffers.

	// Iterate through the image views and create framebuffers from them.
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		
		// The attachments used by this framebuffer (only a color attachment here).
		// Each framebuffer points at exactly one swapchain image view.
		std::array<VkImageView, 2> attachments = {
			swapChainImageViews[i],
			depthImageView
		};

		// Describe how to create a framebuffer compatible with our render pass.
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;																				// Identify the struct type for Vulkan.
		framebufferInfo.renderPass = renderPass;																										// Must be compatible with the subpass/attachments we use.
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());																											// Number of attachments used by this framebuffer.
		framebufferInfo.pAttachments = attachments.data();																										// Pointer to teh attachments used by this framebuffer.
		framebufferInfo.width = swapChainExtent.width;																									// Framebuffer width == swapchain image width.
		framebufferInfo.height = swapChainExtent.height;																								// Framebuffer height == swapchain image height.
		framebufferInfo.layers = 1;																														// Render to a single layer (no stereoscopic/cubemap).

		// Create the framebuffer object; store its handle in our array.
		if (vkCreateFramebuffer(device,&framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

// It manages the memory that is used to store the buffers and command buffers are allocated from them.
void VulkanApplication::createCommandPool() {
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
	
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

// Here is where the command buffer allocates.
void VulkanApplication::createCommandBuffer() {
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void VulkanApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 2> clearValues{}; //= {{{0.05f, 0.0f, 1.0f, 1.0f}}};
	clearValues[0].color = { 0.05f, 0.0f, 1.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	VkBuffer vertexBuffers[] = {vertexBuffer};
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdDrawIndexed(
		commandBuffer, 
		static_cast<uint32_t>(indices.size()), 
		1, 
		0, 
		0,
		0
	);

	vkCmdEndRenderPass(commandBuffer);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void VulkanApplication::createSyncObjects() {
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < static_cast<size_t>(MAX_FRAMES_IN_FLIGHT); i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}

void VulkanApplication::cleanupSwapChain() {
	for (auto framebuffer : swapChainFramebuffers) {
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}

	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	for (auto imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanApplication::recreateSwapChain() {
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createDepthResources();
	createFramebuffers();
}

void VulkanApplication::createVertexBuffer() {
	/*VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(vertices[0]) * vertices.size();
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	 
	if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);*/

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

uint32_t VulkanApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void VulkanApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory?");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanApplication::createIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t) bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VulkanApplication::createDescriptorSetLayout() {
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void VulkanApplication::createUniformBuffers() {
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
		
		vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	}
}

void VulkanApplication::createDepthResources() {
	VkFormat depthFormat = findDepthFormat();

	createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkFormat VulkanApplication::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}

		throw std::runtime_error("failed to find supported format!");
	}
}

VkFormat VulkanApplication::findDepthFormat() {
	return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
								VK_IMAGE_TILING_OPTIMAL,
								VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool VulkanApplication::hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanApplication::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements{};
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS){
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView VulkanApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;

	// The "components" field allows you to swizzle the color channels around. In here it sticks to the default mapping.
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view!");
	}

	return imageView;
}

void VulkanApplication::initVulkan() {
	createInstance();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createDepthResources();
	createFramebuffers();
	createCommandPool();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffer();
	createSyncObjects();
}

void VulkanApplication::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(device);
}

void VulkanApplication::drawFrame() {
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	//vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	updateUniformBuffer(currentFrame);

	// Only reset the fence if we are submitting work.
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	vkResetCommandBuffer(commandBuffers[currentFrame], 0);
	recordCommandBuffer(commandBuffers[currentFrame], imageIndex);

	VkSubmitInfo submitInfo{};  
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanApplication::updateUniformBuffer(uint32_t currentImage) {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	
	ubo.proj[1][1] *= -1;

	memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VulkanApplication::createDescriptorPool() {
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void VulkanApplication::createDescriptorSets() {
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);
	
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;

		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}
}


void VulkanApplication::cleanUp() {
	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);

	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

	vkDestroyRenderPass(device, renderPass, nullptr);

																															// W£ait for GPU to finish before destroying anything
	for (size_t i = 0; i < static_cast<size_t>(MAX_FRAMES_IN_FLIGHT); i++) {
		vkDestroyFence(device, inFlightFences[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);
	
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
	
	vkDestroyDevice(device, nullptr);
	
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();
}