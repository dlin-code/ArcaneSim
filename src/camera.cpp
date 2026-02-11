#define GLM_ENABLE_EXPERIMENTAL

#include "camera.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>


void Camera::processInput(GLFWwindow* window) {

	velocity = glm::vec3(0.0f);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		velocity.z = -1;
	}
	
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		velocity.z = 1;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		velocity.x = -1;
	}
	
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		velocity.x = 1;
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		velocity.y = 1;
	}
	
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		velocity.y = -1;
	}
}

void Camera::processMouse(double xpos, double ypos) {
	// Initialize lastX and lastY
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		return;
	}

	float xoffset = static_cast<float>(xpos - lastX);
	float yoffset = static_cast<float>(lastY - ypos);

	lastX = xpos;
	lastY = ypos;

	float sensitivityX = 0.002f;
	float sensitivityY = 0.002f;
	xoffset *= sensitivityX;
	yoffset *= sensitivityY;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 1.5f) pitch = 1.5f;
	if (pitch < -1.5f) pitch = -1.5f;
}

glm::mat4 Camera::getRotationMatrix() {
	glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3{1.f, 0.f, 0.f});
	glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3{ 0.f, -1.0f, 0.f });

	return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
}

glm::mat4 Camera::getViewMatrix() {
	glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), position);
	glm::mat4 cameraRotation = getRotationMatrix();

	return glm::inverse(cameraTranslation * cameraRotation);
}

void Camera::update() {
	glm::mat4 cameraRotation = getRotationMatrix();
	position += glm::vec3(cameraRotation * glm::vec4(velocity * 0.5f, 0.f));

}
