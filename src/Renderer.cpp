#include "Renderer.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <execution>
#include <iostream>

namespace rtx {

	void Renderer::render(const Scene& scene, const Camera& camera, RenderCanvas* canvas) {
		this->current_camera = &camera;
		this->current_scene = &scene;
		this->width = canvas->getWidth()/* * 2*/;
		this->height = canvas->getHeight()/* * 2*/;

		std::for_each(std::execution::par ,vertitcal_iter.begin(), vertitcal_iter.end(), [this, canvas](int y) {
			for (int x = 0; x < width; x++) {
				Color color = perPixel(x, y);
				accumulated_colors[x + y * width] += color;

				Color accum = accumulated_colors[x + y * width];
				accum /= accumulated_samples;
				accum = glm::clamp(accum, 0.0f, 1.0f);

				canvas->setPixel(x, y, calculateColorFromRGBF(accum.r, accum.g, accum.b));
			}
		});

		/*for (int x = 0; x < canvas->getWidth(); x++) {
			for (int y = 0; y < canvas->getHeight(); y++) {
				
				Color accum_0 = accumulated_colors[x * 2 + y * 2 * width];
				accum_0 /= accumulated_samples;
				Color accum_1 = accumulated_colors[x * 2 + 1 + y * 2 * width];
				accum_1 /= accumulated_samples;
				Color accum_2 = accumulated_colors[x * 2 + (y * 2 + 1) * width];
				accum_2 /= accumulated_samples;
				Color accum_3 = accumulated_colors[x * 2 + 1 + (y * 2 + 1) * width];
				accum_3 /= accumulated_samples;

				Color accum = (accum_0 + accum_1 + accum_2 + accum_3) / 4.0f;
				accum = glm::clamp(accum, 0.0f, 1.0f);

				canvas->setPixel(x, y, calculateColorFromRGBF(accum.r, accum.g, accum.b));
			}
		}*/

		accumulated_samples++;
	}

	void Renderer::resetSamples(int width, int height) {
		accumulated_samples = 1;

		/*width *= 2;
		height *= 2;*/

		if (this->width != width || this->height != height) {
			delete[] accumulated_colors;
			accumulated_colors = new Color[width * height];
		}

		memset(accumulated_colors, 0x00, width * height * sizeof(Color));

		vertitcal_iter.resize(height);
		for (int i = 0; i < height; i++) {
			vertitcal_iter[i] = i;
		}
	}

	static uint32_t PCG_Hash(uint32_t input) {
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}

	float randomFloat(uint32_t& seed) {
		seed = PCG_Hash(seed);
		return float(seed) / float(UINT32_MAX);
	}

	glm::vec3 randomUnitSphere(uint32_t& seed) {
		return glm::normalize(glm::vec3(randomFloat(seed) * 2.0f - 1.0f, randomFloat(seed) * 2.0f - 1.0f, randomFloat(seed) * 2.0f - 1.0f));
	}

	Color Renderer::perPixel(int x, int y) {
		Ray ray;
		ray.origin = current_camera->getPosition();
		ray.direction = current_camera->getRays()[x + y * width];

		glm::vec3 light(0.0f);
		glm::vec3 contribution(1.0f);

		uint32_t seed = x + y * width;
		seed *= accumulated_samples;

		int bounces = 5;
		for (int i = 0; i < bounces; i++) {
			seed += i;

			RayHitPayload payload = traceRay(ray);
			if (!payload.hit) {
				light += current_scene->background_color * contribution;
				break;
			}

			const Material& material = current_scene->materials[payload.sphere->material_idx];

			contribution *= material.albedo;
			light += material.getEmmision();

			ray.origin = payload.hitpoint + payload.normal * 0.0001f;
			ray.direction = glm::normalize(payload.normal + randomUnitSphere(seed));
		}

		return glm::vec4(light, 1.0f);
	}

	Renderer::RayHitPayload Renderer::traceRay(const Ray& ray) {
		const Sphere* closest_sphere = nullptr;
		float closest_t = std::numeric_limits<float>::max();
		for (const Sphere& sphere : current_scene->spheres) {

			glm::vec3 origin = ray.origin - sphere.position;

			float a = glm::dot(ray.direction, ray.direction);
			float b = 2.0f * glm::dot(origin, ray.direction);
			float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;

			float discriminant = b * b - 4.0f * a * c;
			if (discriminant < 0.0f) {
				continue;
			}

			float t0 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
			if (t0 > 0.0f && t0 < closest_t) {
				closest_t = t0;
				closest_sphere = &sphere;
			}

		}

		if (closest_sphere == nullptr) {
			return miss(ray);
		}

		return closestHit(ray, closest_t, closest_sphere);
	}

	Renderer::RayHitPayload Renderer::closestHit(const Ray& ray, float hit_distance, const Sphere* sphere) {
		RayHitPayload payload;
		payload.hit = true;
		payload.sphere = sphere;

		glm::vec3 origin = ray.origin - sphere->position;
		payload.hitpoint = origin + ray.direction * hit_distance;
		payload.normal = glm::normalize(payload.hitpoint);

		payload.hitpoint += sphere->position;

		return payload;
	}

	Renderer::RayHitPayload Renderer::miss(const Ray& ray) {
		RayHitPayload payload;
		payload.hit = false;
		return payload;
	}
}