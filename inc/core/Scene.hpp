#pragma once

#include <vector>

#include "core/Material.hpp"
#include "core/Shapes.hpp"

namespace hzr {

struct Mesh;

struct SceneProperties {
    int32_t width;
    int32_t height;
    int32_t samples;
    std::string output_path;
    std::string model_path;
    float camera_fov;
    Vector3f camera_pos;
    Vector3f camera_target;
};

class Scene {
   public:
    Scene() = default;
    ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    Scene(Scene&&) = delete;
    Scene& operator=(Scene&&) = delete;

    void SetProperties(const SceneProperties& properties) noexcept { m_properties = properties; }
    const SceneProperties& GetProperties() const noexcept { return m_properties; }

    void AddMaterial(const Material& material) noexcept { m_materials.push_back(material); }
    void AddSphere(const Sphere& sphere) noexcept { m_spheres.push_back(sphere); }
    void AddPlane(const Plane& plane) noexcept { m_planes.push_back(plane); }

    void AddMesh(const Mesh& mesh) noexcept;

    const std::vector<Material>& GetMaterials() const noexcept { return m_materials; }
    const std::vector<Sphere>& GetSpheres() const noexcept { return m_spheres; }
    const std::vector<Plane>& GetPlanes() const noexcept { return m_planes; }
    const std::vector<Vertex>& GetVertices() const noexcept { return m_vertices; }
    const std::vector<uint32_t>& GetIndices() const noexcept { return m_indices; }

   private:
    // Scene properties
    SceneProperties m_properties;

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