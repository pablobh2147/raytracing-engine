#include "vulkan/VulkanContext.hpp"

#include <array>
#include <cstring>

#include "core/Logger.hpp"

namespace hzr {

static constexpr std::array<const char*, 1> VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                    VkDebugUtilsMessageTypeFlagsEXT type,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                    void* user_data) {
    if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        Logger::Warning("Vulkan", "{}", callback_data->pMessage);
    }
    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT* create_info,
                                             const VkAllocationCallbacks* allocator,
                                             VkDebugUtilsMessengerEXT* debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, create_info, allocator, debug_messenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
                                          const VkAllocationCallbacks* allocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, allocator);
    }
}

VulkanContext::~VulkanContext() { Destroy(); }

VulkanContext::VulkanContext(VulkanContext&& other) noexcept
    : instance(other.instance),
      debug_messenger(other.debug_messenger),
      physical_device(other.physical_device),
      device(other.device),
      compute_queue(other.compute_queue),
      command_pool(other.command_pool),
      allocator(other.allocator),
      queue_family_indices(other.queue_family_indices),
      validation_layers_enabled(other.validation_layers_enabled) {
    other.instance = VK_NULL_HANDLE;
    other.debug_messenger = VK_NULL_HANDLE;
    other.physical_device = VK_NULL_HANDLE;
    other.device = VK_NULL_HANDLE;
    other.compute_queue = VK_NULL_HANDLE;
    other.command_pool = VK_NULL_HANDLE;
    other.allocator = VK_NULL_HANDLE;
}

VulkanContext& VulkanContext::operator=(VulkanContext&& other) noexcept {
    if (this != &other) {
        Destroy();
        instance = other.instance;
        debug_messenger = other.debug_messenger;
        physical_device = other.physical_device;
        device = other.device;
        compute_queue = other.compute_queue;
        command_pool = other.command_pool;
        allocator = other.allocator;
        queue_family_indices = other.queue_family_indices;
        validation_layers_enabled = other.validation_layers_enabled;

        other.instance = VK_NULL_HANDLE;
        other.debug_messenger = VK_NULL_HANDLE;
        other.physical_device = VK_NULL_HANDLE;
        other.device = VK_NULL_HANDLE;
        other.compute_queue = VK_NULL_HANDLE;
        other.command_pool = VK_NULL_HANDLE;
        other.allocator = VK_NULL_HANDLE;
    }
    return *this;
}

bool VulkanContext::Initialize(const VulkanContextCreateInfo& create_info) {
    if (!CreateInstance(create_info)) {
        Logger::Error("VulkanContext", "Failed to create Vulkan instance");
        return false;
    }

    if (validation_layers_enabled) {
        if (!SetupDebugMessenger()) {
            Logger::Error("VulkanContext", "Failed to setup debug messenger");
            return false;
        }
    }

    if (!PickPhysicalDevice()) {
        Logger::Error("VulkanContext", "Failed to find a suitable GPU");
        return false;
    }

    if (!CreateLogicalDevice()) {
        Logger::Error("VulkanContext", "Failed to create logical device");
        return false;
    }

    if (!CreateCommandPool()) {
        Logger::Error("VulkanContext", "Failed to create command pool");
        return false;
    }

    if (!CreateAllocator()) {
        Logger::Error("VulkanContext", "Failed to create VMA allocator");
        return false;
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    Logger::Info("VulkanContext", "Initialized with device: {}", properties.deviceName);

    return true;
}

void VulkanContext::Destroy() {
    if (device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device);

        if (allocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(allocator);
            allocator = VK_NULL_HANDLE;
        }

        if (command_pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(device, command_pool, nullptr);
            command_pool = VK_NULL_HANDLE;
        }

        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }

    if (debug_messenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
        debug_messenger = VK_NULL_HANDLE;
    }

    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }

    physical_device = VK_NULL_HANDLE;
    compute_queue = VK_NULL_HANDLE;
}

VkPhysicalDeviceProperties VulkanContext::GetDeviceProperties() const {
    VkPhysicalDeviceProperties properties {};
    if (physical_device != VK_NULL_HANDLE) {
        vkGetPhysicalDeviceProperties(physical_device, &properties);
    }
    return properties;
}

uint32_t VulkanContext::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    Logger::Error("VulkanContext", "Failed to find suitable memory type");
    return UINT32_MAX;
}

bool VulkanContext::CreateInstance(const VulkanContextCreateInfo& create_info) {
    validation_layers_enabled = create_info.enable_validation_layers;

    if (validation_layers_enabled && !CheckValidationLayerSupport()) {
        Logger::Warning("VulkanContext", "Validation layers requested but not available, disabling");
        validation_layers_enabled = false;
    }

    VkApplicationInfo app_info {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = create_info.application_name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Hoshizora";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    auto extensions = GetRequiredExtensions(validation_layers_enabled);

    VkInstanceCreateInfo instance_info {};
    instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instance_info.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info {};
    if (validation_layers_enabled) {
        instance_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        instance_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_create_info.pfnUserCallback = DebugCallback;
        instance_info.pNext = &debug_create_info;
    } else {
        instance_info.enabledLayerCount = 0;
        instance_info.pNext = nullptr;
    }

    return vkCreateInstance(&instance_info, nullptr, &instance) == VK_SUCCESS;
}

bool VulkanContext::SetupDebugMessenger() {
    VkDebugUtilsMessengerCreateInfoEXT create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = DebugCallback;

    return CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr, &debug_messenger) == VK_SUCCESS;
}

bool VulkanContext::PickPhysicalDevice() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0) {
        Logger::Error("VulkanContext", "No GPUs with Vulkan support found");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

    int best_score = -1;
    for (const auto& dev : devices) {
        if (IsDeviceSuitable(dev)) {
            int score = RateDeviceSuitability(dev);
            if (score > best_score) {
                best_score = score;
                physical_device = dev;
            }
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        Logger::Error("VulkanContext", "No suitable GPU found");
        return false;
    }

    queue_family_indices = FindQueueFamilies(physical_device);
    return true;
}

bool VulkanContext::CreateLogicalDevice() {
    float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_indices.compute_family.value();
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features {};

    VkDeviceCreateInfo create_info {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = 0;

    if (validation_layers_enabled) {
        create_info.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
        Logger::Error("VulkanContext", "Failed to create logical device");
        return false;
    }

    vkGetDeviceQueue(device, queue_family_indices.compute_family.value(), 0, &compute_queue);
    return true;
}

bool VulkanContext::CreateCommandPool() {
    VkCommandPoolCreateInfo pool_info {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.compute_family.value();

    if (vkCreateCommandPool(device, &pool_info, nullptr, &command_pool) != VK_SUCCESS) {
        Logger::Error("VulkanContext", "Failed to create command pool");
        return false;
    }

    return true;
}

QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice dev) const {
    QueueFamilyIndices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.compute_family = i;
            break;
        }
    }

    return indices;
}

bool VulkanContext::IsDeviceSuitable(VkPhysicalDevice dev) const {
    QueueFamilyIndices indices = FindQueueFamilies(dev);
    return indices.IsComplete();
}

int VulkanContext::RateDeviceSuitability(VkPhysicalDevice dev) const {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(dev, &properties);

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(dev, &mem_properties);

    int score = 0;

    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    } else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 100;
    }

    // Prefer higher compute workgroup sizes
    score += static_cast<int>(properties.limits.maxComputeWorkGroupInvocations / 64);

    return score;
}

bool VulkanContext::CheckValidationLayerSupport() const {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : VALIDATION_LAYERS) {
        bool found = false;
        for (const auto& layer_properties : available_layers) {
            if (std::strcmp(layer_name, layer_properties.layerName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> VulkanContext::GetRequiredExtensions(bool enable_validation) const {
    std::vector<const char*> extensions;

    if (enable_validation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool VulkanContext::CreateAllocator() {
    VmaAllocatorCreateInfo allocator_info {};
    allocator_info.vulkanApiVersion = VK_API_VERSION_1_2;
    allocator_info.physicalDevice = physical_device;
    allocator_info.device = device;
    allocator_info.instance = instance;

    return vmaCreateAllocator(&allocator_info, &allocator) == VK_SUCCESS;
}

VkCommandBuffer VulkanContext::BeginSingleTimeCommands() const {
    VkCommandBufferAllocateInfo alloc_info {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void VulkanContext::EndSingleTimeCommands(VkCommandBuffer command_buffer) const {
    vkEndCommandBuffer(command_buffer);

    VkFenceCreateInfo fence_info {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence fence;
    if (vkCreateFence(device, &fence_info, nullptr, &fence) != VK_SUCCESS) {
        Logger::Error("VulkanContext", "Failed to create fence for command submission");
        vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        return;
    }

    VkSubmitInfo submit_info {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    VkResult submit_result = vkQueueSubmit(compute_queue, 1, &submit_info, fence);
    if (submit_result != VK_SUCCESS) {
        Logger::Error("VulkanContext", "Queue submit failed with error: {}", static_cast<int>(submit_result));
        vkDestroyFence(device, fence, nullptr);
        vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
        return;
    }

    VkResult wait_result = vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (wait_result != VK_SUCCESS) {
        Logger::Error("VulkanContext", "Fence wait failed with error: {}", static_cast<int>(wait_result));
    }

    vkDestroyFence(device, fence, nullptr);
    vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

}  // namespace hzr
