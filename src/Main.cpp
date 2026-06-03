#include <format>
#include <iostream>
#include <string>

#include "Camera.hpp"
#include "ImageBuffer.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"

using namespace rtx;

void ConfigureScene(Scene& scene) noexcept {
    scene.SetBackgroundColor(glm::vec3(0.05f, 0.05f, 0.15f));

    Material red_material;
    red_material.albedo = glm::vec3(0.9f, 0.1f, 0.1f);
    red_material.roughness = 0.2f;
    red_material.metallic = 0.0f;

    Material green_material;
    green_material.albedo = glm::vec3(0.1f, 0.8f, 0.1f);
    green_material.roughness = 0.5f;
    green_material.metallic = 0.3f;

    Material blue_metallic_material;
    blue_metallic_material.albedo = glm::vec3(0.1f, 0.3f, 0.9f);
    blue_metallic_material.roughness = 0.05f;
    blue_metallic_material.metallic = 1.0f;

    Material orange_material;
    orange_material.albedo = glm::vec3(1.0f, 0.5f, 0.1f);
    orange_material.roughness = 0.3f;
    orange_material.metallic = 0.0f;

    Material purple_material;
    purple_material.albedo = glm::vec3(0.7f, 0.1f, 0.8f);
    purple_material.roughness = 0.15f;
    purple_material.metallic = 0.6f;

    Material yellow_material;
    yellow_material.albedo = glm::vec3(0.9f, 0.9f, 0.1f);
    yellow_material.roughness = 0.4f;
    yellow_material.metallic = 0.2f;

    Material cyan_material;
    cyan_material.albedo = glm::vec3(0.1f, 0.8f, 0.8f);
    cyan_material.roughness = 0.1f;
    cyan_material.metallic = 0.8f;

    Material pink_material;
    pink_material.albedo = glm::vec3(1.0f, 0.4f, 0.7f);
    pink_material.roughness = 0.6f;
    pink_material.metallic = 0.0f;

    Material white_material;
    white_material.albedo = glm::vec3(0.9f, 0.9f, 0.9f);
    white_material.roughness = 0.8f;
    white_material.metallic = 0.0f;

    Material light_material;
    light_material.emissive = glm::vec3(1.0f, 0.95f, 0.9f);
    light_material.emissive_strength = 12.0f;

    MaterialHandle red_mat_handle = scene.RegisterMaterial(red_material);
    MaterialHandle green_mat_handle = scene.RegisterMaterial(green_material);
    MaterialHandle blue_mat_handle = scene.RegisterMaterial(blue_metallic_material);
    MaterialHandle orange_mat_handle = scene.RegisterMaterial(orange_material);
    MaterialHandle purple_mat_handle = scene.RegisterMaterial(purple_material);
    MaterialHandle yellow_mat_handle = scene.RegisterMaterial(yellow_material);
    MaterialHandle cyan_mat_handle = scene.RegisterMaterial(cyan_material);
    MaterialHandle pink_mat_handle = scene.RegisterMaterial(pink_material);
    MaterialHandle white_mat_handle = scene.RegisterMaterial(white_material);
    MaterialHandle light_mat_handle = scene.RegisterMaterial(light_material);

    scene.AddSphere(Sphere(glm::vec3(0.0f, 0.0f, -5.0f), 1.0f, red_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(2.5f, 0.0f, -4.0f), 0.8f, green_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(-2.5f, 0.0f, -4.0f), 0.8f, blue_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(1.2f, -0.5f, -6.5f), 0.6f, orange_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(-1.2f, -0.5f, -6.5f), 0.6f, purple_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(4.0f, 0.5f, -6.0f), 0.7f, yellow_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(-4.0f, 0.5f, -6.0f), 0.7f, cyan_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(0.0f, 1.5f, -3.5f), 0.5f, pink_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(3.5f, -0.3f, -8.0f), 0.9f, white_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(-3.5f, -0.3f, -8.0f), 0.9f, red_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(0.0f, -101.0f, 0.0f), 100.0f, white_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(0.0f, 6.0f, -3.0f), 0.6f, light_mat_handle));
    scene.AddSphere(Sphere(glm::vec3(-5.0f, 4.0f, -5.0f), 0.4f, light_mat_handle));
}

int main() {
    constexpr uint32_t OUTPUT_WIDTH = 512;
    constexpr uint32_t OUTPUT_HEIGHT = 512;

    ImageBuffer buffer = ImageBuffer(OUTPUT_WIDTH, OUTPUT_HEIGHT);
    Renderer renderer;

    Camera camera(45.0F, 0.01F, 100.0F);
    camera.SetPosition(glm::vec3(5, 2.5f, 2));
    camera.SetDirection(glm::normalize(glm::vec3(-0.6f, -0.25f, -1.0f)));
    camera.Resize(OUTPUT_WIDTH, OUTPUT_HEIGHT);

    Scene scene;
    ConfigureScene(scene);

    bool render_animation = false;

    RenderOptions options = {
        .samples = 2048,
        .max_bounces = 5,
    };

    if (render_animation) {
        constexpr uint32_t TOTAL_FRAMES = 24;
        constexpr glm::vec3 MOVE_STEP = glm::vec3(0.25F, 0, 0.25F);

        for (uint32_t i = 1; i <= TOTAL_FRAMES; i++) {
            std::cout << "Rendering frame " << i << " of " << TOTAL_FRAMES << std::endl;
            camera.Move(MOVE_STEP);

            renderer.RenderFrame(scene, camera, buffer, options);

            std::string filename = std::format("output/image-{}.png", i);
            WriteImageToDisk(buffer, filename.c_str());
            std::cout << "Rendered frame " << i << " of " << TOTAL_FRAMES << std::endl;
        }
    } else {
        renderer.RenderFrame(scene, camera, buffer, options);
        WriteImageToDisk(buffer, "output/image.png");
    }

    return 0;
}