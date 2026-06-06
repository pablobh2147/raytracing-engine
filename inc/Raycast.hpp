#pragma once

#include "math/Vector.hpp"

namespace hzr {

class Sphere;

struct Raycast {
    Vector3f origin;
    Vector3f direction;
};

struct RaycastResult {
    bool hit = false;
    Vector3f hitpoint = Vector3f(0, 0, 0);
    Vector3f normal = Vector3f(0, 0, 0);
    const Sphere* body = nullptr;
};

}  // namespace hzr
