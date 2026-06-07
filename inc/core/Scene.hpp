#pragma once

#include <span>
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

    void AddMaterial(const Material& material) noexcept { m_materials.push_back(material); }
    void AddSphere(const Sphere& sphere) noexcept { m_spheres.push_back(sphere); }
    void AddPlane(const Plane& plane) noexcept { m_planes.push_back(plane); }
    void AddTriangle(const Triangle& triangle) noexcept { m_triangles.push_back(triangle); }

    void AddMesh(std::span<const Triangle> mesh) noexcept { m_triangles.insert(m_triangles.end(), mesh.begin(), mesh.end()); }

    const std::vector<Material>& GetMaterials() const noexcept { return m_materials; }
    const std::vector<Sphere>& GetSpheres() const noexcept { return m_spheres; }
    const std::vector<Plane>& GetPlanes() const noexcept { return m_planes; }
    const std::vector<Triangle>& GetTriangles() const noexcept { return m_triangles; }

   private:
    // Materials
    std::vector<Material> m_materials;

    // Geometric objects
    std::vector<Sphere> m_spheres;
    std::vector<Plane> m_planes;
    std::vector<Triangle> m_triangles;
};

}  // namespace hzr