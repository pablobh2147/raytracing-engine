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

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <istream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "core/Logger.hpp"
#include "core/Mesh.hpp"
#include "core/Scene.hpp"
#include "loaders/ModelLoader.hpp"
#include "math/Vector.hpp"

namespace hzr {

using PropertyMap = std::unordered_map<std::string, std::string>;

namespace {

using namespace hzr;

PropertyMap LoadProperties(std::istream& input) {
    PropertyMap properties;

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty() || line.find('#') != std::string::npos) {
            continue;
        }

        std::istringstream iss(line);

        std::string key, value;
        std::getline(iss, key, '=');
        std::getline(iss, value);

        if (properties.contains(key)) {
            Logger::Warning("SceneLoader", "Property '{}' already exists, overwriting", key);
        }

        properties.emplace(key, value);
    }
    return properties;
}

template <typename T>
void LoadProperty(const PropertyMap& properties, const std::string& key, T& value, T default_value, std::function<bool(const std::string&, T&)> parser) {
    if (properties.contains(key)) {
        bool valid = parser(properties.at(key), value);
        if (!valid) {
            Logger::Error("SceneLoader", "Invalid value for {}: {}", key, properties.at(key));
        }
        return;
    }

    Logger::Warning("SceneLoader", "Property '{}' not found, using default value {}", key, default_value);
    value = default_value;
}

void LoadIntProperty(const PropertyMap& properties, const std::string& key, int32_t& value, int32_t default_value) {
    LoadProperty<int32_t>(properties, key, value, default_value, [](const std::string& str, int32_t& out) {
        if (str.empty()) {
            return false;
        }

        try {
            out = std::stoi(str);
            return out >= 0;
        } catch (const std::exception& e) {
            return false;
        }
    });
}

void LoadFloatProperty(const PropertyMap& properties, const std::string& key, float& value, float default_value) {
    LoadProperty<float>(properties, key, value, default_value, [](const std::string& str, float& out) {
        if (str.empty()) {
            return false;
        }

        try {
            out = std::stof(str);
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    });
}

void LoadStringProperty(const PropertyMap& properties, const std::string& key, std::string& value, const std::string& default_value) {
    LoadProperty<std::string>(properties, key, value, default_value, [](const std::string& str, std::string& out) {
        out = str;
        return true;
    });
}

void LoadVector3fProperty(const PropertyMap& properties, const std::string& key, Vector3f& value, Vector3f default_value) {
    LoadProperty<Vector3f>(properties, key, value, default_value, [](const std::string& str, Vector3f& out) {
        if (str.empty()) {
            return false;
        }

        std::vector<std::string> parts;
        std::string part;
        std::istringstream iss(str);
        while (std::getline(iss, part, ' ')) {
            parts.push_back(part);
        }

        if (parts.size() != 3) {
            return false;
        }

        try {
            out = Vector3f(std::stof(parts[0]), std::stof(parts[1]), std::stof(parts[2]));
            return true;
        } catch (const std::exception& e) {
            return false;
        }
    });
}

SceneProperties LoadSceneProperties(const PropertyMap& properties, const SceneProperties& default_properties) {
    SceneProperties scene_properties;

    LoadIntProperty(properties, "width", scene_properties.width, default_properties.width);
    LoadIntProperty(properties, "height", scene_properties.height, default_properties.height);
    LoadIntProperty(properties, "samples", scene_properties.samples, default_properties.samples);

    LoadStringProperty(properties, "output_path", scene_properties.output_path, default_properties.output_path);
    LoadStringProperty(properties, "model_path", scene_properties.model_path, default_properties.model_path);

    LoadFloatProperty(properties, "camera_fov", scene_properties.camera_fov, default_properties.camera_fov);
    LoadVector3fProperty(properties, "camera_pos", scene_properties.camera_pos, default_properties.camera_pos);
    LoadVector3fProperty(properties, "camera_target", scene_properties.camera_target, default_properties.camera_target);

    return scene_properties;
}

void LoadHardcodedScene(Scene& scene, const std::filesystem::path& directory, const std::string& model_path) {
    std::optional<Mesh> mesh_opt = LoadObjFromFile(directory / model_path);
    if (mesh_opt) {
        mesh_opt->material_index = 7;  // pink
        scene.AddMesh(*mesh_opt);
    } else {
        Logger::Warning("SceneLoader", "Failed to load 3D model, using hardcoded scene");
    }

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

    scene.AddPlane(Plane(Vector3f(0.0f, 1.0f, 0.0f), -2.5f, 8));  // ground plane
}

}  // namespace

void LoadScene(const std::filesystem::path& directory, Scene& scene) {
    std::ifstream file(directory / "scene.props");
    if (!file.is_open()) {
        Logger::Error("SceneLoader", "Failed to open scene properties file");
        return;
    }

    SceneProperties default_properties = {
        .width = 128,
        .height = 128,
        .samples = 32,
        .output_path = "output.png",
        .model_path = "models/teapot.obj",
        .camera_fov = 90.0f,
        .camera_pos = Vector3f(0.0f, 0.0f, 0.0f),
        .camera_target = Vector3f(0.0f, 0.0f, 1.0f),
    };

    SceneProperties properties = LoadSceneProperties(LoadProperties(file), default_properties);
    scene.SetProperties(properties);

    LoadHardcodedScene(scene, directory, properties.model_path);
}

}  // namespace hzr