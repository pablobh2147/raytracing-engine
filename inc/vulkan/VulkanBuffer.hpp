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
    void Download(void* data, VkDeviceSize size, VkDeviceSize offset = 0) const;

    void* Map();
    void Unmap();

    VkBuffer GetBuffer() const { return buffer; }
    VkDeviceSize GetSize() const { return size; }
    VmaAllocation GetAllocation() const { return allocation; }

   private:
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;
    VkDeviceSize size = 0;
    bool is_mapped = false;
    void* mapped_data = nullptr;
};

}  // namespace hzr
