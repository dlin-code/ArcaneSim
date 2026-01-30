#include "camera.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

void Camera::update() {
	glm::mat4 cameraRotation = getRotationMatrix();
	position += glm::vec3(cameraRotation * glm::vec4(velocity * 0.5f, 0.0f));
}

void Camera::processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		velocity.z = -1;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		velocity.z = 1;
	}
	else {
		velocity.z = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		velocity.x = -1;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		velocity.x = 1;
	}
	else {
		velocity.x = 0;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		velocity.y = 1;
	}
	else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		velocity.y = -1;
	}
	else {
		velocity.y = 0;
	}
}

void Camera::processMouse(float xofffset, float yoffset) {
	
}