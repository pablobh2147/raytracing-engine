#include "vulkan/VulkanBuffer.hpp"

#include <cstring>

#include "core/Logger.hpp"
#include "vulkan/VulkanContext.hpp"

namespace hzr {

VulkanBuffer::~VulkanBuffer() { Destroy(); }

VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
    : buffer(other.buffer),
      allocation(other.allocation),
      allocator(other.allocator),
      size(other.size),
      is_mapped(other.is_mapped),
      mapped_data(other.mapped_data) {
    other.buffer = VK_NULL_HANDLE;
    other.allocation = VK_NULL_HANDLE;
    other.allocator = VK_NULL_HANDLE;
    other.size = 0;
    other.is_mapped = false;
    other.mapped_data = nullptr;
}

VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept {
    if (this != &other) {
        Destroy();
        buffer = other.buffer;
        allocation = other.allocation;
        allocator = other.allocator;
        size = other.size;
        is_mapped = other.is_mapped;
        mapped_data = other.mapped_data;

        other.buffer = VK_NULL_HANDLE;
        other.allocation = VK_NULL_HANDLE;
        other.allocator = VK_NULL_HANDLE;
        other.size = 0;
        other.is_mapped = false;
        other.mapped_data = nullptr;
    }
    return *this;
}

bool VulkanBuffer::Create(const VulkanContext& context, const VulkanBufferCreateInfo& create_info) {
    allocator = context.GetAllocator();
    size = create_info.size;

    VkBufferCreateInfo buffer_info {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = create_info.size;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    switch (create_info.usage) {
        case BufferUsage::StorageBuffer:
            buffer_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case BufferUsage::UniformBuffer:
            buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
        case BufferUsage::StagingBuffer:
            buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            break;
    }

    VmaAllocationCreateInfo alloc_info {};
    if (create_info.host_visible) {
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                           VMA_ALLOCATION_CREATE_MAPPED_BIT;
    } else {
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    }

    VkResult result = vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &buffer, &allocation, nullptr);
    if (result != VK_SUCCESS) {
        Logger::Error("VulkanBuffer", "Failed to create buffer");
        return false;
    }

    return true;
}

void VulkanBuffer::Destroy() {
    if (is_mapped) {
        Unmap();
    }

    if (buffer != VK_NULL_HANDLE && allocator != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, buffer, allocation);
        buffer = VK_NULL_HANDLE;
        allocation = VK_NULL_HANDLE;
    }

    allocator = VK_NULL_HANDLE;
    size = 0;
}

void VulkanBuffer::Upload(const void* data, VkDeviceSize upload_size, VkDeviceSize offset) {
    void* mapped = Map();
    if (mapped) {
        std::memcpy(static_cast<char*>(mapped) + offset, data, upload_size);
        Unmap();
    }
}

void VulkanBuffer::Download(void* data, VkDeviceSize download_size, VkDeviceSize offset) const {
    void* mapped = nullptr;
    if (vmaMapMemory(allocator, allocation, &mapped) == VK_SUCCESS) {
        std::memcpy(data, static_cast<const char*>(mapped) + offset, download_size);
        vmaUnmapMemory(allocator, allocation);
    }
}

void* VulkanBuffer::Map() {
    if (is_mapped) {
        return mapped_data;
    }

    if (vmaMapMemory(allocator, allocation, &mapped_data) == VK_SUCCESS) {
        is_mapped = true;
        return mapped_data;
    }

    Logger::Error("VulkanBuffer", "Failed to map buffer memory");
    return nullptr;
}

void VulkanBuffer::Unmap() {
    if (is_mapped) {
        vmaUnmapMemory(allocator, allocation);
        is_mapped = false;
        mapped_data = nullptr;
    }
}

}  // namespace hzr
