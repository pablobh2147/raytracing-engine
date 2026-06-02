#include <chrono>
#include <iostream>
#include <string>

#include "Camera.h"
#include "ImageBuffer.hpp"
#include "Renderer.h"

std::chrono::milliseconds start_time;

void startTimer() {
    start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
}

void stopTimer() {
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    std::cout << "Time: " << ms.count() - start_time.count() << "ms" << std::endl;
}

int main() {
    constexpr int width = 512;
    constexpr int height = 512;

    rtx::ImageBuffer canvas = rtx::ImageBuffer(width, height);
    rtx::Renderer renderer;
    Camera camera(45.0f, 0.01f, 100.0f);
    camera.setPosition(glm::vec3(15, 10, 15));
    camera.setDirection(glm::vec3(-1, -0.6, -1));
    camera.resize(width, height);

    Scene scene;

    scene.background_color = glm::vec3(0.6f, 0.7f, 0.9f);

    Material material_1;
    material_1.albedo = glm::vec3(1.0f, 0.0f, 1.0f);
    material_1.roughness = 0.04f;

    scene.materials.push_back(material_1);
    scene.spheres.push_back(Sphere(glm::vec3(0, 0.2f, 0), 1.2f, 0));

    Material material_2;
    material_2.albedo = glm::vec3(0.0f, 0.0f, 1.0f);
    material_2.roughness = 0.6f;

    scene.materials.push_back(material_2);
    scene.spheres.push_back(Sphere(glm::vec3(0.0f, -201.0, 0.0f), 200.0f, 1));

    Material material_3;
    material_3.albedo = glm::vec3(1.0f, 1.0f, 0.0f);
    material_3.roughness = 0.05f;
    material_3.emissive = glm::vec3(245 / 255.0f, 167 / 255.0f, 66 / 255.0f);
    material_3.emissive_strength = 40.0f;

    scene.materials.push_back(material_3);
    scene.spheres.push_back(Sphere(glm::vec3(100.0f, 0.5f, -300), 100, 2));

    bool render_animation = false;
    constexpr int samples = 128;

    if (render_animation) {
        constexpr int total_frames = 24 * 3;

        for (int i = 0; i < total_frames; i++) {
            camera.move(glm::vec3(0.25f / 2.0f, 0, 0.25f / 2.0f));

            renderer.resetSamples(width, height);
            for (int s = 1; s <= samples; s++) {
                startTimer();
                renderer.render(scene, camera, &canvas);
                stopTimer();
            }

            std::string filename = std::string("output/image-") + std::to_string(i) + std::string(".png");
            rtx::WriteImageToDisk(canvas, filename.c_str());
            std::cout << "Rendered frame " << i << " of " << total_frames << std::endl;
        }
    } else {
        renderer.resetSamples(width, height);
        for (int i = 1; i <= samples; i++) {
            startTimer();
            renderer.render(scene, camera, &canvas);
            stopTimer();

            int progress = static_cast<int>((static_cast<float>(i) / samples) * 100);
            std::cout << "\rProgress: " << progress << "%" << std::flush;
        }
        std::cout << std::endl;
        rtx::WriteImageToDisk(canvas, "output/image.png");
    }

    return 0;
}