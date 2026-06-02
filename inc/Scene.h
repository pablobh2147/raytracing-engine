#pragma once

#include "Sphere.h"
#include <vector>

struct Scene {

	glm::vec3 background_color = glm::vec3(0.0f, 0.0f, 0.0f);
	std::vector<Sphere> spheres;
	std::vector<Material> materials;

};