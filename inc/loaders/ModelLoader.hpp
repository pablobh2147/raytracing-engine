#pragma once

#include <filesystem>
#include <optional>

#include "core/Mesh.hpp"

namespace hzr {

std::optional<Mesh> LoadObjFromFile(const std::filesystem::path& file_path);

}  // namespace hzr