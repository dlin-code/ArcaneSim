#pragma once
#include <glm/glm.hpp>

struct Particle {
	alignas(16) glm::vec4 position;
	alignas(16) glm::vec4 velocity;
	alignas(16) float lifeTime;
};