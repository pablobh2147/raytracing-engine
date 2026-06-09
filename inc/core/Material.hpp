#pragma once

#include "math/Vector.hpp"

namespace hzr {

struct alignas(16) Material {
    Vector3f albedo;
    float roughness = 1.0f;
    Vector3f emission;
    float metallic = 0.0f;

    Material() = default;
    Material(const Vector3f& albedo, float roughness, const Vector3f& emission, float metallic) : albedo(albedo), roughness(roughness), emission(emission), metallic(metallic) {}
};

}  // namespace hzr
