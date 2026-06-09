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