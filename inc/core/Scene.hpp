#pragma once

#include <vector>

#include "core/Material.hpp"
#include "core/Shapes.hpp"

namespace hzr {

class Scene {
   public:
    Scene() = default;
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

   private:
    // Materials
    std::vector<Material> m_materials;

    // Geometric objects
    std::vector<Sphere> m_spheres;
    std::vector<Plane> m_planes;
    std::vector<Triangle> m_triangles;
};

}  // namespace hzr