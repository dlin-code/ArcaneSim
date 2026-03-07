#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
	glm::vec3 velocity;
	glm::vec3 position{0.f, 0.f, 5.f};

	// vertical position;
	float pitch{ 0.f };
	// horizontal rotation
	float yaw{ 0.f };

	double lastX{ 400.0 };
	double lastY{ 300.0 };
	bool firstMouse{ true };

	glm::mat4 getViewMatrix();
	glm::mat4 getRotationMatrix();

	void processInput(GLFWwindow* window);
	void processMouse(double xpos, double ypos);

	void update();
};