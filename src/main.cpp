#include <iostream>																				// iostream: For printing messages.
#include <stdexcept>																			// stdexcept: For throwing and catching errors.
#include <cstdlib>																				// cstdlib: For EXIT_SUCCESS / EXIT_FAILURE codes.
#include "VulkanApp.h"

#include <cstdint>		// Necessary for uint32_t
#include <limits>		// Necessary for std::numeric_limits





int main() {
	
	//GLFWwindow* window;
	/*VkPhysicalDevice physicalDevice;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;*/
	VulkanApplication vulkanApp;
	const std::vector<char> code;
	//VkCommandBuffer commandBuffer;
	uint32_t imageIndex = 0;

	try {
		vulkanApp.initWindow(/*window*/);																									// Creates the window.
		vulkanApp.initVulkan(/*window*//*, physicalDevice, graphicsQueue, presentQueue, swapChain, */code, imageIndex);

		std::cout << "Vulkan instance created successfully!\n";  

		//while (!glfwWindowShouldClose(window)) {																						// Runs a basic event loop.
		//	glfwPollEvents();
		//	vulkanApp.drawFrame();
		//}
		vulkanApp.mainLoop(/*window*/);
		vulkanApp.cleanUp(/*window*//*, swapChain*/);

		return EXIT_SUCCESS;
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}