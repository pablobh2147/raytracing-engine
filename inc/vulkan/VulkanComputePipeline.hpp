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

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace hzr {

class VulkanContext;

struct DescriptorBinding {
    uint32_t binding;
    VkDescriptorType type;
    VkShaderStageFlags stage_flags = VK_SHADER_STAGE_COMPUTE_BIT;
};

struct ComputePipelineCreateInfo {
    std::string shader_path;
    std::vector<DescriptorBinding> bindings;
    uint32_t push_constant_size = 0;
};

class VulkanComputePipeline {
   public:
    VulkanComputePipeline() = default;
    ~VulkanComputePipeline();

    VulkanComputePipeline(const VulkanComputePipeline&) = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;
    VulkanComputePipeline(VulkanComputePipeline&&) noexcept;
    VulkanComputePipeline& operator=(VulkanComputePipeline&&) noexcept;

    bool Create(const VulkanContext& context, const ComputePipelineCreateInfo& create_info);
    void Destroy();

    VkPipeline GetPipeline() const { return pipeline; }
    VkPipelineLayout GetPipelineLayout() const { return pipeline_layout; }
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptor_set_layout; }
    VkDescriptorPool GetDescriptorPool() const { return descriptor_pool; }

    VkDescriptorSet AllocateDescriptorSet();
    void UpdateDescriptorSet(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset = 0);

    void Dispatch(VkCommandBuffer cmd, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);
    void PushConstants(VkCommandBuffer cmd, const void* data, uint32_t size);

   private:
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    std::vector<char> ReadShaderFile(const std::string& path);

   private:
    VkDevice device = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    uint32_t push_constant_size = 0;
};

}  // namespace hzr
