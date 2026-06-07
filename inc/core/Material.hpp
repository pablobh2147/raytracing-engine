#pragma once

#include "math/Vector.hpp"

namespace hzr {

struct Material {
    Vector3f albedo;
    float roughness = 1.0f;
    float metallic = 0.0f;
    Vector3f emission;
};

}  // namespace hzr
