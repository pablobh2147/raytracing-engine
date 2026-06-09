/*  ==============================================================================
 *  Hoshizora (星空) — Physically Based Renderer
 *  ------------------------------------------------------------------------------
 *  Copyright (c) 2026 Pablo Bermejo Hernández. All rights reserved.
 *
 *  This software is licensed under the MIT License.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 *  See the LICENSE file in the project root for full license information.
 *  ==============================================================================
 */

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