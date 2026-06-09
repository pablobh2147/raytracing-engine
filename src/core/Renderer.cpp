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

#include "core/Renderer.hpp"

#include <vulkan/vulkan_core.h>

#include "core/Logger.hpp"
#include "shared/binding-constants.h"
#include "shared/compute-constants.h"
#include "vulkan/VulkanBuffer.hpp"
#include "vulkan/VulkanContext.hpp"

namespace hzr {

bool Renderer::Initialize(const RendererConfig& config) noexcept {
    m_config = config;

    Logger::Info("Renderer", "Initializing renderer...");

    if (!m_context.Initialize()) {
        Logger::Error("Renderer", "Failed to initialize Vulkan context");
        return false;
    }

    Logger::Info("Renderer", "Vulkan context initialized");

    if (!CreateOutputBuffer()) {
        Logger::Error("Renderer", "Failed to create output buffer");
        return false;
    }

    Logger::Info("Renderer", "Output buffer created");

    if (!CreateComputePipeline()) {
        Logger::Error("Renderer", "Failed to create compute pipeline");
        return false;
    }

    Logger::Info("Renderer", "Compute pipeline created");

    m_descriptor_set = m_compute_pipeline.AllocateDescriptorSet();
    if (m_descriptor_set == VK_NULL_HANDLE) {
        Logger::Error("Renderer", "Failed to allocate descriptor set");
        return false;
    }

    Logger::Info("Renderer", "Descriptor set allocated");

    return true;
}

void Renderer::Destroy() noexcept {
    m_output_buffer.Destroy();

    m_sphere_buffer.Destroy();
    m_plane_buffer.Destroy();
    m_vertex_buffer.Destroy();
    m_index_buffer.Destroy();
    m_material_buffer.Destroy();

    m_compute_pipeline.Destroy();
    m_context.Destroy();
}

void Renderer::SetScene(std::shared_ptr<Scene> scene) noexcept {
    m_scene = std::move(scene);
}

bool Renderer::BakeScene() noexcept {
    if (!m_scene) {
        Logger::Error("Renderer", "No scene set");
        return false;
    }

    Logger::Info("Renderer", "Creating geometry buffers...");
    if (!CreateGeometryBuffers(*m_scene)) {
        Logger::Error("Renderer", "Failed to create geometry buffers");
        return false;
    }
    Logger::Info("Renderer", "Geometry buffers created");

    if (!CreateMaterialBuffer(*m_scene)) {
        Logger::Error("Renderer", "Failed to create material buffer");
        return false;
    }

    if (!UpdateDescriptorSets()) {
        Logger::Error("Renderer", "Failed to update descriptor sets");
        return false;
    }
    Logger::Info("Renderer", "Descriptor sets updated");

    return true;
}

bool Renderer::CreateOutputBuffer() noexcept {
    VulkanBufferCreateInfo buffer_info = {};
    buffer_info.size = m_config.width * m_config.height * sizeof(uint32_t);
    buffer_info.usage = BufferUsage::StorageBuffer;
    buffer_info.host_visible = true;

    return m_output_buffer.Create(m_context, buffer_info);
}

template <typename T>
inline bool CreateBuffer(VulkanContext& ctx, VulkanBuffer& buffer, const std::vector<T>& data) {
    VulkanBufferCreateInfo buffer_info = {};
    buffer_info.size = data.size() * sizeof(T);
    buffer_info.usage = BufferUsage::StorageBuffer;
    buffer_info.host_visible = true;

    if (!buffer.Create(ctx, buffer_info)) {
        return false;
    }

    buffer.Upload(data.data(), buffer_info.size);
    return true;
}

bool Renderer::CreateGeometryBuffers(const Scene& scene) noexcept {
    const std::vector<Sphere>& spheres = scene.GetSpheres();
    const std::vector<Plane>& planes = scene.GetPlanes();
    const std::vector<Vertex>& vertices = scene.GetVertices();
    const std::vector<uint32_t>& indices = scene.GetIndices();

    Logger::Info("Renderer", "Creating geometry buffers...");

    if (!CreateBuffer(m_context, m_sphere_buffer, spheres)) {
        Logger::Error("Renderer", "Failed to create sphere buffer!");
        return false;
    }

    if (!CreateBuffer(m_context, m_plane_buffer, planes)) {
        Logger::Error("Renderer", "Failed to create plane buffer!");
        return false;
    }

    if (!CreateBuffer(m_context, m_vertex_buffer, vertices)) {
        Logger::Error("Renderer", "Failed to create vertex buffer!");
        return false;
    }

    if (!CreateBuffer(m_context, m_index_buffer, indices)) {
        Logger::Error("Renderer", "Failed to create index buffer!");
        return false;
    }

    Logger::Info("Renderer", "Geometry data created successfully!");

    return true;
}

bool Renderer::CreateMaterialBuffer(const Scene& scene) noexcept {
    const std::vector<Material>& materials = scene.GetMaterials();

    if (materials.empty()) {
        Logger::Warning("Renderer", "No materials to create buffer for!");
        return true;
    }

    if (!CreateBuffer(m_context, m_material_buffer, materials)) {
        Logger::Error("Renderer", "Failed to create material buffer");
        return false;
    }

    return true;
}

void Renderer::Render(const Camera& camera) noexcept {
    uint32_t group_x = (m_config.width + COMPUTE_WORKGROUP_SIZE - 1) / COMPUTE_WORKGROUP_SIZE;
    uint32_t group_y = (m_config.height + COMPUTE_WORKGROUP_SIZE - 1) / COMPUTE_WORKGROUP_SIZE;

    // Record and submit compute commands
    VkCommandBuffer cmd = m_context.BeginSingleTimeCommands();

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_compute_pipeline.GetPipeline());
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_compute_pipeline.GetPipelineLayout(), 0, 1, &m_descriptor_set, 0, nullptr);

    PushConstants pc {m_config.width, m_config.height, m_config.samples, camera.GetView(), camera.GetProjection()};
    m_compute_pipeline.PushConstants(cmd, &pc, sizeof(PushConstants));

    vkCmdDispatch(cmd, group_x, group_y, 1);

    m_context.EndSingleTimeCommands(cmd);
}

bool Renderer::ReadImage(std::vector<uint32_t>& pixels) noexcept {
    return m_output_buffer.Download(pixels.data(), pixels.size() * sizeof(uint32_t));
}

bool Renderer::CreateComputePipeline() noexcept {
    ComputePipelineCreateInfo pipeline_info = {};
    pipeline_info.shader_path = COMPUTE_SHADER_PATH;
    pipeline_info.bindings = {
        {.binding = BINDING_OUTPUT,    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {.binding = BINDING_MATERIALS, .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {.binding = BINDING_SPHERES,   .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {.binding = BINDING_PLANES,    .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {.binding = BINDING_VERTICES,  .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {.binding = BINDING_INDICES,   .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
    };
    pipeline_info.push_constant_size = sizeof(PushConstants);

    return m_compute_pipeline.Create(m_context, pipeline_info);
}

bool Renderer::UpdateDescriptorSets() noexcept {
    // Frame buffers
    m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, BINDING_OUTPUT, m_output_buffer.GetBuffer(), m_output_buffer.GetSize());

    // Scene data
    m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, BINDING_MATERIALS, m_material_buffer.GetBuffer(), m_material_buffer.GetSize());

    m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, BINDING_SPHERES, m_sphere_buffer.GetBuffer(), m_sphere_buffer.GetSize());
    m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, BINDING_PLANES, m_plane_buffer.GetBuffer(), m_plane_buffer.GetSize());
    m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, BINDING_VERTICES, m_vertex_buffer.GetBuffer(), m_vertex_buffer.GetSize());
    m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, BINDING_INDICES, m_index_buffer.GetBuffer(), m_index_buffer.GetSize());

    return true;
}

}  // namespace hzr