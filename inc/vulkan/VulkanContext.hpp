#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace hzr {

struct QueueFamilyIndices {
    std::optional<uint32_t> compute_family;

    bool IsComplete() const { return compute_family.has_value(); }
};

struct VulkanContextCreateInfo {
    std::string application_name = "Hoshizora";
    bool enable_validation_layers = true;
};

class VulkanContext {
   public:
    VulkanContext() = default;
    ~VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&) noexcept;
    VulkanContext& operator=(VulkanContext&&) noexcept;

    bool Initialize(const VulkanContextCreateInfo& create_info = {});
    void Destroy();

    VkInstance GetInstance() const { return instance; }
    VkPhysicalDevice GetPhysicalDevice() const { return physical_device; }
    VkDevice GetDevice() const { return device; }
    VkQueue GetComputeQueue() const { return compute_queue; }
    uint32_t GetComputeQueueFamily() const { return queue_family_indices.compute_family.value(); }
    VkCommandPool GetCommandPool() const { return command_pool; }
    VmaAllocator GetAllocator() const { return allocator; }

    VkPhysicalDeviceProperties GetDeviceProperties() const;

    uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

    VkCommandBuffer BeginSingleTimeCommands() const;
    void EndSingleTimeCommands(VkCommandBuffer command_buffer) const;

   private:
    bool CreateInstance(const VulkanContextCreateInfo& create_info);
    bool SetupDebugMessenger();
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateCommandPool();
    bool CreateAllocator();

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
    bool IsDeviceSuitable(VkPhysicalDevice device) const;
    int RateDeviceSuitability(VkPhysicalDevice device) const;

    bool CheckValidationLayerSupport() const;
    std::vector<const char*> GetRequiredExtensions(bool enable_validation) const;

   private:
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue compute_queue = VK_NULL_HANDLE;
    VkCommandPool command_pool = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;

    QueueFamilyIndices queue_family_indices;
    bool validation_layers_enabled = false;
};

}  // namespace hzr
