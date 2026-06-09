#pragma once

#include <memory>
#include <string_view>

#include "core/Camera.hpp"
#include "core/Scene.hpp"
#include "math/Matrix.hpp"
#include "vulkan/VulkanBuffer.hpp"
#include "vulkan/VulkanComputePipeline.hpp"
#include "vulkan/VulkanContext.hpp"

namespace hzr {

struct RendererConfig {
    uint32_t width;
    uint32_t height;
    uint32_t samples;
};

struct PushConstants {
    uint32_t width;
    uint32_t height;
    uint32_t samples;
    alignas(16) Matrix4f view;
    alignas(16) Matrix4f projection;
};

class Renderer {
   private:
    static constexpr std::string_view COMPUTE_SHADER_PATH = "build/shaders/raytracer.comp.spv";

   public:
    static constexpr uint32_t COLOR_COMPONENTS = 4;

   public:
    Renderer() = default;
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    bool Initialize(const RendererConfig& config) noexcept;
    void Destroy() noexcept;

    void SetScene(std::shared_ptr<Scene> scene) noexcept;
    bool BakeScene() noexcept;

    void Render(const Camera& camera) noexcept;

    bool ReadImage(std::vector<uint32_t>& pixels) noexcept;

   private:
    bool CreateOutputBuffer() noexcept;

    bool CreateGeometryBuffers(const Scene& scene) noexcept;
    bool CreateMaterialBuffer(const Scene& scene) noexcept;

    bool CreateComputePipeline() noexcept;
    bool UpdateDescriptorSets() noexcept;

   private:
    // Scene
    std::shared_ptr<Scene> m_scene;

    RendererConfig m_config;

    // Vulkan context
    VulkanContext m_context;

    VulkanComputePipeline m_compute_pipeline;
    VkDescriptorSet m_descriptor_set;

    // Output buffer
    VulkanBuffer m_output_buffer;

    // Scene buffers
    VulkanBuffer m_material_buffer;

    VulkanBuffer m_sphere_buffer;
    VulkanBuffer m_plane_buffer;
    VulkanBuffer m_vertex_buffer;
    VulkanBuffer m_index_buffer;
};

}  // namespace hzr