#pragma once

#include <glm/glm.hpp>

namespace rtx {

struct Material {
    glm::vec3 albedo;
    float roughness = 1.0f;
    float metallic = 0.0f;

    glm::vec3 emissive = glm::vec3(0.0f);
    float emissive_strength = 1.0f;

    glm::vec3 emmision() const {
        return emissive * emissive_strength;
    }
};

}  // namespace rtx
