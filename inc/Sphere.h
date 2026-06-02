#pragma once

#include "Material.h"
#include <glm/glm.hpp>

struct Sphere {

	glm::vec3 position;
	float radius;
	int material_idx;

	Sphere(const glm::vec3& position, float radius, int material_idx) : position(position), radius(radius), material_idx(material_idx) {}

};