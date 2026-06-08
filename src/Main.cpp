#include <stb_image_write.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>

#include "core/Camera.hpp"
#include "core/Logger.hpp"
#include "core/Renderer.hpp"
#include "core/Scene.hpp"
#include "core/Shapes.hpp"
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

void LoadObj(std::istream& input, Scene& scene, uint32_t material_index) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(1000);
    indices.reserve(1000);

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(Vertex(Vector3f(x, y, z), material_index));
        } else if (command == "f") {
            uint32_t a, b, c;
            iss >> a >> b >> c;
            indices.push_back(a - 1);
            indices.push_back(b - 1);
            indices.push_back(c - 1);
        }
    }

    scene.AddMesh(vertices, indices);
}

void LoadScene(Scene& scene) {
    std::ifstream obj_file("examples/models/cow.obj");
    LoadObj(obj_file, scene, 2);
    obj_file.close();

    scene.AddMaterial(Material(Vector3f(0.9f, 0.1f, 0.1f), 0.2f, Vector3f(0.0f), 0.0f));                       // 0: red
    scene.AddMaterial(Material(Vector3f(0.1f, 0.8f, 0.1f), 0.5f, Vector3f(0.0f), 0.3f));                       // 1: green
    scene.AddMaterial(Material(Vector3f(0.1f, 0.3f, 0.9f), 0.05f, Vector3f(0.0f), 1.0f));                      // 2: blue metallic
    scene.AddMaterial(Material(Vector3f(1.0f, 0.5f, 0.1f), 0.3f, Vector3f(0.0f), 0.0f));                       // 3: orange
    scene.AddMaterial(Material(Vector3f(0.7f, 0.1f, 0.8f), 0.15f, Vector3f(0.0f), 0.6f));                      // 4: purple
    scene.AddMaterial(Material(Vector3f(0.9f, 0.9f, 0.1f), 0.4f, Vector3f(0.0f), 0.2f));                       // 5: yellow
    scene.AddMaterial(Material(Vector3f(0.1f, 0.8f, 0.8f), 0.1f, Vector3f(0.0f), 0.8f));                       // 6: cyan
    scene.AddMaterial(Material(Vector3f(1.0f, 0.4f, 0.7f), 0.9f, Vector3f(0.0f), 0.1f));                       // 7: pink
    scene.AddMaterial(Material(Vector3f(0.9f, 0.9f, 0.9f), 0.8f, Vector3f(0.0f), 0.0f));                       // 8: white
    scene.AddMaterial(Material(Vector3f(1.0f, 1.0f, 1.0f), 0.0f, Vector3f(1.0f, 0.95f, 0.9f) * 12.0f, 0.0f));  // 9: light

    scene.AddSphere(Sphere(Vector3f(0.0f, 0.0f, -5.0f), 1.0f, 0));    // center red
    scene.AddSphere(Sphere(Vector3f(2.5f, 0.0f, -4.0f), 0.8f, 1));    // right green
    scene.AddSphere(Sphere(Vector3f(-2.5f, 0.0f, -4.0f), 0.8f, 2));   // left blue metallic
    scene.AddSphere(Sphere(Vector3f(1.2f, -0.5f, -6.5f), 0.6f, 3));   // orange
    scene.AddSphere(Sphere(Vector3f(-1.2f, -0.5f, -6.5f), 0.6f, 4));  // purple
    scene.AddSphere(Sphere(Vector3f(4.0f, 0.5f, -6.0f), 0.7f, 5));    // yellow
    scene.AddSphere(Sphere(Vector3f(-4.0f, 0.5f, -6.0f), 0.7f, 6));   // cyan
    scene.AddSphere(Sphere(Vector3f(0.0f, 1.5f, -3.5f), 0.5f, 7));    // pink
    scene.AddSphere(Sphere(Vector3f(3.5f, -0.3f, -8.0f), 0.9f, 8));   // white
    scene.AddSphere(Sphere(Vector3f(-3.5f, -0.3f, -8.0f), 0.9f, 0));  // red
    scene.AddSphere(Sphere(Vector3f(0.0f, 6.0f, -3.0f), 0.6f, 9));    // light 1
    scene.AddSphere(Sphere(Vector3f(-5.0f, 6.0f, -5.0f), 0.4f, 9));   // light 2

    scene.AddPlane(Plane(Vector3f(0.0f, 1.0f, 0.0f), -1.5f, 8));  // ground plane
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

    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    Logger::Info("main", "Loading scene...");
    LoadScene(*scene);
    Logger::Info("main", "Scene loaded");

    Renderer renderer;
    if (!renderer.Initialize(renderer_config)) {
        Logger::Error("main", "Failed to initialize renderer");
        return 1;
    }

    Camera camera(config.camera_fov, 0.01F, 100.0F);
    camera.CalculateView(config.camera_pos, config.camera_target, Vector3f(0, 1, 0));
    float aspect = static_cast<float>(renderer_config.width) / static_cast<float>(renderer_config.height);
    camera.CalculateProjection(aspect);

    Logger::Info("main", "Loading scene...");
    renderer.SetScene(scene);
    if (!renderer.BakeScene()) {
        Logger::Error("main", "Failed to bake scene");
        return 1;
    }
    Logger::Info("main", "Scene loaded");

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