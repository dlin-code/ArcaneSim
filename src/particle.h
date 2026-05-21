#pragma once
#include <glm/glm.hpp>

struct Particle {
	alignas(16) glm::vec3 position;
	alignas(16) glm::vec3 velocity;
	alignas(16) float lifeTime;
};