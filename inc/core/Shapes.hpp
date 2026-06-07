#pragma once

#include <cstdint>

#include "math/Vector.hpp"

namespace hzr {

struct alignas(16) Sphere {
    Vector3f position;
    float radius;
    uint32_t material_index;
};

struct alignas(16) Plane {
    Vector3f normal;
    float distance;
    uint32_t material_index;
};

struct Triangle {
    Vector3f v1;
    uint32_t material_index;
    alignas(16) Vector3f v2;
    alignas(16) Vector3f v3;

    Triangle() = default;
    Triangle(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3, uint32_t material_index = 0) : v1(v1), material_index(material_index), v2(v2), v3(v3) {}
};

}  // namespace hzr