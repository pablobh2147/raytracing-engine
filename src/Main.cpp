#include <stb_image_write.h>

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

#include "core/Camera.hpp"
#include "core/Logger.hpp"
#include "core/Renderer.hpp"
#include "core/Scene.hpp"
#include "math/Vector.hpp"

using namespace hzr;

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

void LoadScene(Scene& scene) {
    scene.AddTriangle(Triangle(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(1.0f, 0.0f, 0.0f), 2));
    scene.AddTriangle(Triangle(Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(1.0f, 1.0f, 0.0f), 2));
}

int main() {
    constexpr uint32_t COLOR_COMPONENTS = 4;
    constexpr uint32_t COMPUTE_GROUP_SIZE = 16;

    SceneConfig config;
    LoadSceneConfig(config);

    RendererConfig renderer_config;
    renderer_config.width = config.width;
    renderer_config.height = config.height;
    renderer_config.samples = config.samples;

    Renderer renderer;
    if (!renderer.Initialize(renderer_config)) {
        Logger::Error("main", "Failed to initialize renderer");
        return 1;
    }

    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    LoadScene(*scene);

    Camera camera(config.camera_fov, 0.01F, 100.0F);
    camera.CalculateView(config.camera_pos, config.camera_target, Vector3f(0, 1, 0));
    float aspect = static_cast<float>(renderer_config.width) / static_cast<float>(renderer_config.height);
    camera.CalculateProjection(aspect);

    renderer.SetScene(scene);
    renderer.BakeScene();

    Logger::Info("main", "Rendering...");
    renderer.Render(camera);
    Logger::Info("main", "Finished rendering");

    // Read back the buffer
    std::vector<uint32_t> pixels(renderer_config.width * renderer_config.height);
    if (!renderer.ReadImage(pixels)) {
        Logger::Error("main", "Failed to read image");
        return 1;
    }

    // Write to PNG using stb_image_write
    stbi_write_png(config.output_path.c_str(), renderer_config.width, renderer_config.height, COLOR_COMPONENTS, pixels.data(), renderer_config.width * COLOR_COMPONENTS);

    Logger::Info("main", "Wrote image to {}", config.output_path);

    return 0;
}