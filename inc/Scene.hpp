#pragma once

#include <vector>

#include "Material.hpp"
#include "Sphere.hpp"

namespace rtx {

using MaterialHandle = size_t;

class Scene {
   private:
    glm::vec3 m_background_color = glm::vec3(0.0f, 0.0f, 0.0f);
    std::vector<Sphere> m_spheres;
    std::vector<Material> m_materials;

   public:
    const glm::vec3& GetBackgroundColor() const { return m_background_color; }
    void SetBackgroundColor(const glm::vec3& color) { m_background_color = color; }

    const std::vector<Sphere>& GetSpheres() const { return m_spheres; }

    MaterialHandle AddSphere(const Sphere& sphere) {
        m_spheres.push_back(sphere);
        return m_spheres.size() - 1;
    }

    const std::vector<Material>& GetMaterials() const { return m_materials; }
    const Material& GetMaterial(MaterialHandle handle) const { return m_materials[handle]; }

    MaterialHandle RegisterMaterial(const Material& material) {
        m_materials.push_back(material);
        return m_materials.size() - 1;
    }
};

}  // namespace rtx
