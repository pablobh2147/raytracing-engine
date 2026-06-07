#include <stb_image_write.h>

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "core/Camera.hpp"
#include "core/Logger.hpp"
#include "math/Matrix.hpp"
#include "math/Vector.hpp"
#include "vulkan/VulkanBuffer.hpp"
#include "vulkan/VulkanComputePipeline.hpp"
#include "vulkan/VulkanContext.hpp"

using namespace hzr;

struct PushConstants {
    uint32_t width;
    uint32_t height;
    uint32_t samples;
    alignas(16) Matrix4f view;
    alignas(16) Matrix4f projection;
};

struct SceneConfig {
    uint32_t width;
    uint32_t height;
    uint32_t samples;
    std::string output_path;

    float camera_fov;
    Vector3f camera_pos;
    Vector3f camera_target;
};

void LoadSceneConfig(SceneConfig& config) {
    std::cin >> config.width >> config.height;
    std::cin >> config.samples;
    std::cin >> config.output_path;
    std::cin >> config.camera_fov;
    std::cin >> config.camera_pos.x >> config.camera_pos.y >> config.camera_pos.z;
    std::cin >> config.camera_target.x >> config.camera_target.y >> config.camera_target.z;
}

int main() {
    constexpr uint32_t COLOR_COMPONENTS = 4;
    constexpr uint32_t COMPUTE_GROUP_SIZE = 16;

    SceneConfig config;
    LoadSceneConfig(config);

    uint32_t width = config.width;
    uint32_t height = config.height;
    float aspect = static_cast<float>(width) / static_cast<float>(height);

    uint32_t samples = config.samples;

    std::string output_path = config.output_path;

    // Initialize Vulkan
    VulkanContext ctx;
    if (!ctx.Initialize()) {
        Logger::Error("main", "Failed to initialize Vulkan");
        return 1;
    }

    // Create output buffer (host-visible so we can read it back)
    VulkanBuffer output_buffer;
    VulkanBufferCreateInfo buffer_info = {};
    buffer_info.size = width * height * sizeof(uint32_t);
    buffer_info.usage = BufferUsage::StorageBuffer;
    buffer_info.host_visible = true;

    if (!output_buffer.Create(ctx, buffer_info)) {
        Logger::Error("main", "Failed to create output buffer");
        return 1;
    }

    // Create compute pipeline
    VulkanComputePipeline pipeline;
    ComputePipelineCreateInfo pipeline_info = {};
    pipeline_info.shader_path = "build/shaders/raytracer.comp.spv";
    pipeline_info.bindings = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}
    };
    pipeline_info.push_constant_size = sizeof(PushConstants);

    if (!pipeline.Create(ctx, pipeline_info)) {
        Logger::Error("main", "Failed to create compute pipeline");
        return 1;
    }

    // Allocate and update descriptor set
    VkDescriptorSet descriptor_set = pipeline.AllocateDescriptorSet();
    pipeline.UpdateDescriptorSet(descriptor_set, 0, output_buffer.GetBuffer(), output_buffer.GetSize());

    Camera camera(config.camera_fov, 0.01F, 100.0F);
    camera.CalculateView(config.camera_pos, config.camera_target, Vector3f(0, 1, 0));
    camera.CalculateProjection(aspect);

    uint32_t group_x = (width + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    uint32_t group_y = (height + COMPUTE_GROUP_SIZE - 1) / COMPUTE_GROUP_SIZE;
    std::vector<uint32_t> pixels(width * height);

    // Record and submit compute commands
    VkCommandBuffer cmd = ctx.BeginSingleTimeCommands();

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.GetPipeline());
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.GetPipelineLayout(), 0, 1, &descriptor_set, 0, nullptr);

    PushConstants pc {width, height, samples, camera.GetView(), camera.GetProjection()};
    pipeline.PushConstants(cmd, &pc, sizeof(PushConstants));

    vkCmdDispatch(cmd, group_x, group_y, 1);

    ctx.EndSingleTimeCommands(cmd);

    // Read back the buffer
    output_buffer.Download(pixels.data(), pixels.size() * sizeof(uint32_t));

    // Write to PNG using stb_image_write
    stbi_write_png(output_path.c_str(), width, height, COLOR_COMPONENTS, pixels.data(), width * COLOR_COMPONENTS);

    Logger::Info("main", "Finished rendering");

    // Cleanup
    pipeline.Destroy();
    output_buffer.Destroy();
    ctx.Destroy();

    return 0;
}