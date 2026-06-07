#pragma once

#include "math/Vector.hpp"

namespace hzr {

struct Sphere {
    Vector3f position;
    float radius;
    uint32_t material_index;
};

struct Plane {
    Vector3f normal;
    float distance;
    uint32_t material_index;
};

struct Triangle {
    Vector3f v1;
    Vector3f v2;
    Vector3f v3;
    uint32_t material_index;
};

}  // namespace hzr