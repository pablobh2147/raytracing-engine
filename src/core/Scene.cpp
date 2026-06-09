#include "core/Scene.hpp"

#include "core/Mesh.hpp"

namespace hzr {

void Scene::AddMesh(const Mesh& mesh) noexcept {
    const uint32_t base_index = static_cast<uint32_t>(m_vertices.size());

    auto vertex_it = m_vertices.insert(m_vertices.end(), mesh.vertices.begin(), mesh.vertices.end());
    auto index_it = m_indices.insert(m_indices.end(), mesh.indices.begin(), mesh.indices.end());

    // Update material indices
    for (; index_it != m_indices.end(); ++index_it) {
        *index_it += base_index;
    }

    for (; vertex_it != m_vertices.end(); ++vertex_it) {
        vertex_it->material_index = mesh.material_index;
    }
}

}  // namespace hzr