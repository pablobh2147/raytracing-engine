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

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace hzr {

class VulkanContext;

enum class BufferUsage {
    StorageBuffer,
    UniformBuffer,
    StagingBuffer,
};

struct VulkanBufferCreateInfo {
    VkDeviceSize size = 0;
    BufferUsage usage = BufferUsage::StorageBuffer;
    bool host_visible = false;
};

class VulkanBuffer {
   public:
    VulkanBuffer() = default;
    ~VulkanBuffer();

    VulkanBuffer(const VulkanBuffer&) = delete;
    VulkanBuffer& operator=(const VulkanBuffer&) = delete;
    VulkanBuffer(VulkanBuffer&&) noexcept;
    VulkanBuffer& operator=(VulkanBuffer&&) noexcept;

    bool Create(const VulkanContext& context, const VulkanBufferCreateInfo& create_info);
    void Destroy();

    void Upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);
    bool Download(void* data, VkDeviceSize size, VkDeviceSize offset = 0) const;

    void* Map();
    void Unmap();

    VkBuffer GetBuffer() const { return buffer; }
    VkDeviceSize GetSize() const { return size; }
    VmaAllocation GetAllocation() const { return allocation; }

    bool IsMapped() const { return is_mapped; }
    void* GetMappedData() const { return mapped_data; }

   private:
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    bool is_mapped = false;
    void* mapped_data = nullptr;
};

}  // namespace hzr
