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