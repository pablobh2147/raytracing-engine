#pragma once

#include <cstddef>
#include <cstdint>

#include "math/Vector.hpp"

namespace hzr {

constexpr size_t STD430_ALIGNMENT = 16;

struct alignas(STD430_ALIGNMENT) Sphere {
    Vector3f position;
    float radius;
    uint32_t material_index;

    Sphere() = default;
    Sphere(const Vector3f& position, float radius, uint32_t material_index) : position(position), radius(radius), material_index(material_index) {}
};

struct alignas(STD430_ALIGNMENT) Plane {
    Vector3f normal;
    float distance;
    uint32_t material_index;

    Plane() = default;
    Plane(const Vector3f& normal, float distance, uint32_t material_index) : normal(normal), distance(distance), material_index(material_index) {}
};

struct alignas(STD430_ALIGNMENT) Vertex {
    Vector3f position;
    uint32_t material_index;

    Vertex() = default;
    Vertex(const Vector3f& position, uint32_t material_index) : position(position), material_index(material_index) {}
};

}  // namespace hzr