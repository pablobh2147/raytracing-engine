#include "core/Renderer.hpp"

#include "core/Logger.hpp"
#include "vulkan/VulkanBuffer.hpp"

namespace hzr {

bool Renderer::Initialize(const RendererConfig& config) noexcept {
    m_config = config;

    if (!m_context.Initialize()) {
        Logger::Error("Renderer", "Failed to initialize Vulkan context");
        return false;
    }

    if (!CreateOutputBuffer()) {
        Logger::Error("Renderer", "Failed to create output buffer");
        return false;
    }

    if (!CreateComputePipeline()) {
        Logger::Error("Renderer", "Failed to create compute pipeline");
        return false;
    }

    m_descriptor_set = m_compute_pipeline.AllocateDescriptorSet();
    if (m_descriptor_set == VK_NULL_HANDLE) {
        Logger::Error("Renderer", "Failed to allocate descriptor set");
        return false;
    }

    return true;
}

void Renderer::Destroy() noexcept {
    m_output_buffer.Destroy();

    m_triangle_buffer.Destroy();
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

    if (!CreateGeometryBuffers(*m_scene)) {
        Logger::Error("Renderer", "Failed to create geometry buffers");
        return false;
    }

    // if (!CreateMaterialBuffer(*m_scene)) {
    //     Logger::Error("Renderer", "Failed to create material buffer");
    //     return false;
    // }

    if (!UpdateDescriptorSets()) {
        Logger::Error("Renderer", "Failed to update descriptor sets");
        return false;
    }

    return true;
}

bool Renderer::CreateOutputBuffer() noexcept {
    VulkanBufferCreateInfo buffer_info = {};
    buffer_info.size = m_config.width * m_config.height * sizeof(uint32_t);
    buffer_info.usage = BufferUsage::StorageBuffer;
    buffer_info.host_visible = true;

    return m_output_buffer.Create(m_context, buffer_info);
}

bool Renderer::CreateGeometryBuffers(const Scene& scene) noexcept {
    const auto& triangles = scene.GetTriangles();

    VulkanBufferCreateInfo triangle_buffer_info = {};
    triangle_buffer_info.size = sizeof(Triangle) * triangles.size();
    triangle_buffer_info.usage = BufferUsage::StorageBuffer;
    triangle_buffer_info.host_visible = true;

    if (!m_triangle_buffer.Create(m_context, triangle_buffer_info)) {
        Logger::Error("Renderer", "Failed to create triangle buffer");
        return false;
    }

    m_triangle_buffer.Upload(triangles.data(), triangles.size() * sizeof(Triangle));

    return true;
}

bool Renderer::CreateMaterialBuffer(const Scene& scene) noexcept {
    const auto& materials = scene.GetMaterials();

    VulkanBufferCreateInfo material_buffer_info = {};
    material_buffer_info.size = sizeof(Material) * materials.size();
    material_buffer_info.usage = BufferUsage::StorageBuffer;
    material_buffer_info.host_visible = true;

    if (!m_material_buffer.Create(m_context, material_buffer_info)) {
        Logger::Error("Renderer", "Failed to create material buffer");
        return false;
    }

    m_material_buffer.Upload(materials.data(), materials.size() * sizeof(Material));

    return true;
}

void Renderer::Render(const Camera& camera) noexcept {
    uint32_t group_x = (m_config.width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    uint32_t group_y = (m_config.height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;

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
    pipeline_info.shader_path = "build/shaders/raytracer.comp.spv";
    pipeline_info.bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        //{2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
    };
    pipeline_info.push_constant_size = sizeof(PushConstants);

    return m_compute_pipeline.Create(m_context, pipeline_info);
}

bool Renderer::UpdateDescriptorSets() noexcept {
    m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, 0, m_output_buffer.GetBuffer(), m_output_buffer.GetSize());

    // m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, 2, m_material_buffer.GetBuffer(), m_material_buffer.GetSize());
    m_compute_pipeline.UpdateDescriptorSet(m_descriptor_set, 1, m_triangle_buffer.GetBuffer(), m_triangle_buffer.GetSize());

    return true;
}

}  // namespace hzr