#pragma once

#include <filesystem>

namespace hzr {

class Scene;

void LoadScene(const std::filesystem::path& directory, Scene& scene);

}  // namespace hzr