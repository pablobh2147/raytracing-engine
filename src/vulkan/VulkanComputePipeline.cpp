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

#include "vulkan/VulkanComputePipeline.hpp"

#include <format>
#include <fstream>

#include "core/Logger.hpp"
#include "vulkan/VulkanContext.hpp"

namespace hzr {

VulkanComputePipeline::~VulkanComputePipeline() { Destroy(); }

VulkanComputePipeline::VulkanComputePipeline(VulkanComputePipeline&& other) noexcept
    : device(other.device),
      pipeline(other.pipeline),
      pipeline_layout(other.pipeline_layout),
      descriptor_set_layout(other.descriptor_set_layout),
      descriptor_pool(other.descriptor_pool),
      push_constant_size(other.push_constant_size) {
    other.device = VK_NULL_HANDLE;
    other.pipeline = VK_NULL_HANDLE;
    other.pipeline_layout = VK_NULL_HANDLE;
    other.descriptor_set_layout = VK_NULL_HANDLE;
    other.descriptor_pool = VK_NULL_HANDLE;
}

VulkanComputePipeline& VulkanComputePipeline::operator=(VulkanComputePipeline&& other) noexcept {
    if (this != &other) {
        Destroy();
        device = other.device;
        pipeline = other.pipeline;
        pipeline_layout = other.pipeline_layout;
        descriptor_set_layout = other.descriptor_set_layout;
        descriptor_pool = other.descriptor_pool;
        push_constant_size = other.push_constant_size;

        other.device = VK_NULL_HANDLE;
        other.pipeline = VK_NULL_HANDLE;
        other.pipeline_layout = VK_NULL_HANDLE;
        other.descriptor_set_layout = VK_NULL_HANDLE;
        other.descriptor_pool = VK_NULL_HANDLE;
    }
    return *this;
}

bool VulkanComputePipeline::Create(const VulkanContext& context, const ComputePipelineCreateInfo& create_info) {
    device = context.GetDevice();
    push_constant_size = create_info.push_constant_size;

    // Create descriptor set layout
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
    layout_bindings.reserve(create_info.bindings.size());

    for (const auto& binding : create_info.bindings) {
        VkDescriptorSetLayoutBinding layout_binding {};
        layout_binding.binding = binding.binding;
        layout_binding.descriptorType = binding.type;
        layout_binding.descriptorCount = 1;
        layout_binding.stageFlags = binding.stage_flags;
        layout_bindings.push_back(layout_binding);
    }

    VkDescriptorSetLayoutCreateInfo layout_info {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());
    layout_info.pBindings = layout_bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
        Logger::Error("VulkanComputePipeline", "Failed to create descriptor set layout");
        return false;
    }

    // Create descriptor pool
    std::vector<VkDescriptorPoolSize> pool_sizes;
    for (const auto& binding : create_info.bindings) {
        VkDescriptorPoolSize pool_size {};
        pool_size.type = binding.type;
        pool_size.descriptorCount = 16;
        pool_sizes.push_back(pool_size);
    }

    VkDescriptorPoolCreateInfo pool_info {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_info.pPoolSizes = pool_sizes.data();
    pool_info.maxSets = 16;

    if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
        Logger::Error("VulkanComputePipeline", "Failed to create descriptor pool");
        return false;
    }

    // Create pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &descriptor_set_layout;

    VkPushConstantRange push_constant_range {};
    if (push_constant_size > 0) {
        push_constant_range.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = push_constant_size;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;
    }

    if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
        Logger::Error("VulkanComputePipeline", "Failed to create pipeline layout");
        return false;
    }

    // Load shader and create pipeline
    auto shader_code = ReadShaderFile(create_info.shader_path);
    if (shader_code.empty()) {
        Logger::Error("VulkanComputePipeline", "Failed to read shader: {}", create_info.shader_path.c_str());
        return false;
    }

    VkShaderModule shader_module = CreateShaderModule(shader_code);
    if (shader_module == VK_NULL_HANDLE) {
        Logger::Error("VulkanComputePipeline", "Failed to create shader module");
        return false;
    }

    VkPipelineShaderStageCreateInfo stage_info {};
    stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stage_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    stage_info.module = shader_module;
    stage_info.pName = "main";

    VkComputePipelineCreateInfo pipeline_info {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipeline_info.stage = stage_info;
    pipeline_info.layout = pipeline_layout;

    VkResult result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);

    vkDestroyShaderModule(device, shader_module, nullptr);

    if (result != VK_SUCCESS) {
        Logger::Error("VulkanComputePipeline", "Failed to create compute pipeline");
        return false;
    }

    return true;
}

void VulkanComputePipeline::Destroy() {
    if (device == VK_NULL_HANDLE) return;

    if (pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }

    if (pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }

    if (descriptor_pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
        descriptor_pool = VK_NULL_HANDLE;
    }

    if (descriptor_set_layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
        descriptor_set_layout = VK_NULL_HANDLE;
    }

    device = VK_NULL_HANDLE;
}

VkDescriptorSet VulkanComputePipeline::AllocateDescriptorSet() {
    VkDescriptorSetAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptor_set_layout;

    VkDescriptorSet descriptor_set;
    if (vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set) != VK_SUCCESS) {
        Logger::Error("VulkanComputePipeline", "Failed to allocate descriptor set");
        return VK_NULL_HANDLE;
    }

    return descriptor_set;
}

void VulkanComputePipeline::UpdateDescriptorSet(VkDescriptorSet set, uint32_t binding, VkBuffer buffer, VkDeviceSize size, VkDeviceSize offset) {
    VkDescriptorBufferInfo buffer_info {};
    buffer_info.buffer = buffer;
    buffer_info.offset = offset;
    buffer_info.range = size;

    VkWriteDescriptorSet write {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = set;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
}

void VulkanComputePipeline::Dispatch(VkCommandBuffer cmd, uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) {
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdDispatch(cmd, group_count_x, group_count_y, group_count_z);
}

void VulkanComputePipeline::PushConstants(VkCommandBuffer cmd, const void* data, uint32_t size) {
    vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, size, data);
}

VkShaderModule VulkanComputePipeline::CreateShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
        Logger::Error("VulkanComputePipeline", "Failed to create shader module");
        return VK_NULL_HANDLE;
    }

    return shader_module;
}

std::vector<char> VulkanComputePipeline::ReadShaderFile(const std::string& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        Logger::Error("VulkanComputePipeline", "Failed to open shader file: {}", path);
        return {};
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}

}  // namespace hzr
