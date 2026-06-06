#pragma once

#include "math/Vector.hpp"

namespace hzr {

struct Sphere {
    Vector3f position;
    float radius;
    int material_idx;

    Sphere(const Vector3f& position, float radius, int material_idx)
        : position(position),
          radius(radius),
          material_idx(material_idx) {}
};

}  // namespace hzr