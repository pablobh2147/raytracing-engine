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

    void AddMesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices) noexcept;

    const std::vector<Material>& GetMaterials() const noexcept { return m_materials; }
    const std::vector<Sphere>& GetSpheres() const noexcept { return m_spheres; }
    const std::vector<Plane>& GetPlanes() const noexcept { return m_planes; }
    const std::vector<Vertex>& GetVertices() const noexcept { return m_vertices; }
    const std::vector<uint32_t>& GetIndices() const noexcept { return m_indices; }

   private:
    // Materials
    std::vector<Material> m_materials;

    // Geometric objects
    std::vector<Sphere> m_spheres;
    std::vector<Plane> m_planes;

    // Mesh data
    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
};

}  // namespace hzr