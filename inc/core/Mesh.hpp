#pragma once

#include <cstdint>
#include <vector>

#include "core/Shapes.hpp"

namespace hzr {

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    uint32_t material_index;
};

}  // namespace hzr