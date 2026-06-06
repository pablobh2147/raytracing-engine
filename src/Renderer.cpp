#include "Renderer.hpp"

#include <algorithm>
#include <cmath>
#include <execution>
#include <iostream>

#include "math/Vector.hpp"

namespace hzr {

void Renderer::RenderFrame(const Scene& scene, const Camera& camera, ImageBuffer& buffer, const RenderOptions& options) {
    ResetSamples(buffer.GetWidth(), buffer.GetHeight());
    for (uint32_t i = 1; i <= options.samples; i++) {
        std::cout << "Sample " << i << " of " << options.samples << std::endl;
        RenderSample(scene, camera, buffer, options);
    }
}

void Renderer::RenderSample(const Scene& scene, const Camera& camera, ImageBuffer& buffer, const RenderOptions& options) {
    this->current_camera = &camera;
    this->current_scene = &scene;
    this->width = buffer.GetWidth();
    this->height = buffer.GetHeight();

    const uint32_t total_pixels = width * height;
    std::for_each(std::execution::par, pixel_iter.begin(),
                  pixel_iter.end(), [this, &buffer, &options](uint32_t pixel_idx) {
                      uint32_t x = pixel_idx % width;
                      uint32_t y = pixel_idx / width;

                      Color color = ProcessFragment(x, y, options);
                      accumulated_colors[pixel_idx] += color;

                      Color accum = accumulated_colors[pixel_idx];
                      accum /= accumulated_samples;
                      accum = glm::clamp(accum, 0.0F, 1.0F);
                      Vector3f rgb = glm::pow(Vector3f(accum), Vector3f(1.0F / 2.2F));

                      buffer.SetPixel(x, y, calculateColorFromRGBF(rgb.r, rgb.g, rgb.b));
                  });

    accumulated_samples++;
}

void Renderer::ResetSamples(uint32_t width, uint32_t height) {
    accumulated_samples = 1;

    const uint32_t total_pixels = width * height;
    accumulated_colors.assign(total_pixels, Color(0.0F));

    pixel_iter.resize(total_pixels);
    for (uint32_t i = 0; i < total_pixels; i++) {
        pixel_iter[i] = i;
    }
}

static uint32_t PCG_Hash(uint32_t input) {
    uint32_t state = input * 747796405u + 2891336453u;
    uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float RandomFloat(uint32_t& seed) {
    seed = PCG_Hash(seed);
    return float(seed) / float(UINT32_MAX);
}

Vector3f RandomUnitSphere(uint32_t& seed) {
    Vector3f v;
    do {
        v = Vector3f(
            RandomFloat(seed) * 2.0F - 1.0F,
            RandomFloat(seed) * 2.0F - 1.0F,
            RandomFloat(seed) * 2.0F - 1.0F);
    } while (glm::dot(v, v) > 1.0F || glm::dot(v, v) < 1e-6F);
    return glm::normalize(v);
}

Color Renderer::ProcessFragment(uint32_t x, uint32_t y, const RenderOptions& options) {
    Raycast ray = {
        .origin = current_camera->GetPosition(),
        .direction = current_camera->GetRayDirection(x, y),
    };

    Vector3f light(0.0F);
    Vector3f contribution(1.0F);

    uint32_t seed = x + y * width;
    seed *= accumulated_samples;

    const uint32_t bounces = options.max_bounces;

    for (uint32_t i = 0; i < bounces; i++) {
        seed += i;

        RaycastResult result = TraceRay(ray);
        if (!result.hit) {
            light += current_scene->GetBackgroundColor() * contribution;
            break;
        }

        const Material& material = current_scene->GetMaterial(result.body->material_idx);

        light += material.emission() * contribution;

        Vector3f specular_dir = glm::reflect(ray.direction, result.normal);
        Vector3f diffuse_dir = glm::normalize(result.normal + RandomUnitSphere(seed));
        float roughness2 = material.roughness * material.roughness;

        ray.origin = result.hitpoint + result.normal * 0.0001F;
        ray.direction = glm::normalize(glm::mix(specular_dir, diffuse_dir, roughness2));

        Vector3f specular_tint = glm::mix(Vector3f(1.0F), material.albedo, material.metallic);
        contribution *= glm::mix(specular_tint, material.albedo, roughness2);

        if (i >= 3) {
            float p = glm::max(contribution.r, glm::max(contribution.g, contribution.b));
            if (RandomFloat(seed) > p) {
                break;
            }
            contribution /= p;
        }
    }

    return glm::vec4(light, 1.0F);
}

RaycastResult Renderer::TraceRay(const Raycast& ray) {
    const Sphere* closest_sphere = nullptr;
    float closest_t = std::numeric_limits<float>::max();
    for (const Sphere& sphere : current_scene->GetSpheres()) {
        Vector3f origin = ray.origin - sphere.position;

        float half_b = glm::dot(origin, ray.direction);
        float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;

        float discriminant = half_b * half_b - c;
        if (discriminant < 0.0F) {
            continue;
        }

        float t0 = -half_b - glm::sqrt(discriminant);
        if (t0 > 0.0F && t0 < closest_t) {
            closest_t = t0;
            closest_sphere = &sphere;
        }
    }

    if (closest_sphere == nullptr) {
        return RaycastResult();
    }

    return ClosestHit(ray, closest_t, closest_sphere);
}

RaycastResult Renderer::ClosestHit(const Raycast& ray, float hit_distance, const Sphere* sphere) {
    RaycastResult result;
    result.hit = true;
    result.body = sphere;

    Vector3f origin = ray.origin - sphere->position;
    result.hitpoint = origin + ray.direction * hit_distance;
    result.normal = glm::normalize(result.hitpoint);

    result.hitpoint += sphere->position;

    return result;
}

}  // namespace hzr