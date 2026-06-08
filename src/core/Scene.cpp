#include "core/Scene.hpp"

namespace hzr {

void Scene::AddMesh(std::span<const Vertex> vertices, std::span<const uint32_t> indices) noexcept {
    const uint32_t base_index = static_cast<uint32_t>(m_vertices.size());

    m_vertices.insert(m_vertices.end(), vertices.begin(), vertices.end());
    m_indices.insert(m_indices.end(), indices.begin(), indices.end());

    // Update material indices
    for (uint32_t i = 0; i < indices.size(); i++) {
        m_indices[base_index + i] += base_index;
    }
}

}  // namespace hzr