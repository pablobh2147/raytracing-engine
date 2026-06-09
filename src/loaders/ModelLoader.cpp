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

#include "loaders/ModelLoader.hpp"

#include <fstream>
#include <sstream>

#include "core/Mesh.hpp"

namespace hzr {

uint32_t ReadIndex(std::istream& input) {
    std::string token;
    std::getline(input, token, ' ');
    size_t slash_pos = token.find('/');
    if (slash_pos != std::string::npos) {
        token = token.substr(0, slash_pos);
    }
    uint32_t index = std::stoul(token);
    return index - 1;
}

bool LoadObj(std::istream& input, Mesh& mesh) {
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
        std::getline(iss, command, ' ');

        if (command == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back(Vertex(Vector3f(x, y, z), 0));
        } else if (command == "f") {
            indices.push_back(ReadIndex(iss));
            indices.push_back(ReadIndex(iss));
            indices.push_back(ReadIndex(iss));
        }
    }

    mesh.vertices = std::move(vertices);
    mesh.indices = std::move(indices);

    return true;
}

std::optional<Mesh> LoadObjFromFile(const std::filesystem::path& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return std::nullopt;
    }

    Mesh mesh;
    if (!LoadObj(file, mesh)) {
        return std::nullopt;
    }

    return mesh;
}

}  // namespace hzr