#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
	glm::vec3 velocity;
	glm::vec3 position;
	// vertical position;
	float pitch { 0.f };
	// horizontal rotation
	float yaw { 0.f };


	glm::mat4 getViewMatrix();
	glm::mat4 getRotationMatrix();

	void processInput(GLFWwindow* window/*, float deltaTime*/);
	void processMouse(GLFWwindow* window, float xoffset, float yoffset);

	

	void update();
};