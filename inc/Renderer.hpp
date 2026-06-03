#pragma once

#include "Camera.hpp"
#include "ImageBuffer.hpp"
#include "Raycast.hpp"
#include "Scene.hpp"

namespace rtx {

struct RenderOptions {
    uint32_t samples = 1;
    uint32_t max_bounces = 1;
};

class Renderer {
   public:
    void RenderFrame(const Scene& scene, const Camera& camera, ImageBuffer& buffer, const RenderOptions& options = {1});

   private:
    void RenderSample(const Scene& scene, const Camera& camera, ImageBuffer& buffer, const RenderOptions& options);
    void ResetSamples(uint32_t width, uint32_t height);

   private:
    RaycastResult TraceRay(const Raycast& ray);
    RaycastResult ClosestHit(const Raycast& ray, float hit_distance, const Sphere* sphere);

    Color ProcessFragment(uint32_t x, uint32_t y, const RenderOptions& options);

    const Scene* current_scene = nullptr;
    const Camera* current_camera = nullptr;

    uint32_t width;
    uint32_t height;

    std::vector<Color> accumulated_colors;
    uint32_t accumulated_samples = 1;

    std::vector<uint32_t> pixel_iter;
};
}  // namespace rtx