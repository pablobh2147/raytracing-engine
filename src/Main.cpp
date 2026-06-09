/*  ==============================================================================
 *  Hoshizora (星空) — Physically Based Renderer
 *  ------------------------------------------------------------------------------
 *  Copyright (c) 2026 Pablo Bermejo Hernández. All rights reserved.
 *
 *  This software is licensed under the MIT License.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *
 *  See the LICENSE file in the project root for full license information.
 *  ==============================================================================
 */

#include <stb_image_write.h>

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "core/Camera.hpp"
#include "core/Logger.hpp"
#include "core/Renderer.hpp"
#include "core/Scene.hpp"
#include "loaders/SceneLoader.hpp"
#include "math/Vector.hpp"

using namespace hzr;

void RenderScene(const std::string& scene_path_str) {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();

    std::filesystem::path scene_path = scene_path_str;

    Logger::Info("main", "Loading scene from {}", scene_path_str);
    LoadScene(scene_path, *scene);
    Logger::Info("main", "Scene loaded");

    const SceneProperties& properties = scene->GetProperties();

    RendererConfig renderer_config;
    renderer_config.width = properties.width;
    renderer_config.height = properties.height;
    renderer_config.samples = properties.samples;

    Renderer renderer;
    if (!renderer.Initialize(renderer_config)) {
        Logger::Error("main", "Failed to initialize renderer");
        return;
    }

    Camera camera(properties.camera_fov, 0.01F, 100.0F);
    camera.CalculateView(properties.camera_pos, properties.camera_target, Vector3f(0, 1, 0));
    float aspect = static_cast<float>(renderer_config.width) / static_cast<float>(renderer_config.height);
    camera.CalculateProjection(aspect);

    Logger::Info("main", "Loading scene...");
    renderer.SetScene(scene);
    if (!renderer.BakeScene()) {
        Logger::Error("main", "Failed to bake scene");
        return;
    }
    Logger::Info("main", "Scene loaded");

    Logger::Info("main", "Rendering...");
    renderer.Render(camera);
    Logger::Info("main", "Finished rendering");

    // Read back the buffer
    std::vector<uint32_t> pixels(renderer_config.width * renderer_config.height);
    if (!renderer.ReadImage(pixels)) {
        Logger::Error("main", "Failed to read image");
        return;
    }

    // Write to PNG using stb_image_write
    stbi_write_png(properties.output_path.c_str(), renderer_config.width, renderer_config.height, Renderer::COLOR_COMPONENTS, pixels.data(), renderer_config.width * Renderer::COLOR_COMPONENTS);

    Logger::Info("main", "Wrote image to {}", properties.output_path);
}

void PrintHelp(const char* program_name) {
    std::cout << "Usage: " << program_name << " <scene_path>" << std::endl;
}

int main(int argc, char** argv) {
    const char* program_name = "Hoshizora";

    if (argc >= 1) {
        program_name = argv[0];
    }

    if (argc >= 2 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
        PrintHelp(program_name);
        return 0;
    }

    if (argc <= 1) {
        PrintHelp(program_name);
        return 1;
    }

    std::string scene_path_str = argv[1];
    RenderScene(scene_path_str);

    return 0;
}