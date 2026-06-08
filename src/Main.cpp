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

void LoadObj(std::istream& input, Scene& scene) {
    std::vector<Vector3f> vertices;
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
            vertices.push_back(Vector3f(x, y, z));
        } else if (command == "f") {
            uint32_t a, b, c;
            iss >> a >> b >> c;
            indices.push_back(a - 1);
            indices.push_back(b - 1);
            indices.push_back(c - 1);
        }
    }

    std::vector<Triangle> triangles;
    triangles.reserve(indices.size() / 3);

    for (size_t i = 0; i < indices.size(); i += 3) {
        triangles.push_back(Triangle(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]], 2));
    }

    scene.AddMesh(triangles);
}

void LoadScene(Scene& scene) {
    std::ifstream obj_file("examples/models/cow.obj");
    LoadObj(obj_file, scene);
    obj_file.close();

    scene.AddSphere(Sphere(Vector3f(0.0, 0.0, -5.0), 1.0, 0));    // center red
    scene.AddSphere(Sphere(Vector3f(2.5, 0.0, -4.0), 0.8, 1));    // right green
    scene.AddSphere(Sphere(Vector3f(-2.5, 0.0, -4.0), 0.8, 2));   // left blue metallic
    scene.AddSphere(Sphere(Vector3f(1.2, -0.5, -6.5), 0.6, 3));   // orange
    scene.AddSphere(Sphere(Vector3f(-1.2, -0.5, -6.5), 0.6, 4));  // purple
    scene.AddSphere(Sphere(Vector3f(4.0, 0.5, -6.0), 0.7, 5));    // yellow
    scene.AddSphere(Sphere(Vector3f(-4.0, 0.5, -6.0), 0.7, 6));   // cyan
    scene.AddSphere(Sphere(Vector3f(0.0, 1.5, -3.5), 0.5, 7));    // pink
    scene.AddSphere(Sphere(Vector3f(3.5, -0.3, -8.0), 0.9, 8));   // white
    scene.AddSphere(Sphere(Vector3f(-3.5, -0.3, -8.0), 0.9, 0));  // red
    scene.AddSphere(Sphere(Vector3f(0.0, 6.0, -3.0), 0.6, 9));    // light 1
    scene.AddSphere(Sphere(Vector3f(-5.0, 6.0, -5.0), 0.4, 9));   // light 2

    scene.AddPlane(Plane(Vector3f(0.0, 1.0, 0.0), -1.5, 8));  // ground plane
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