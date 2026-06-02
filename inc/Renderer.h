#pragma once

#include "Camera.h"
#include "RenderCanvas.h"
#include "Scene.h"
#include "Ray.h"

namespace rtx {

	class Renderer {

	public:
		void render(const Scene& scene, const Camera& camera, RenderCanvas* canvas);

		void resetSamples(int width, int height);

	private:
		struct RayHitPayload {
			bool hit;
			glm::vec3 hitpoint;
			glm::vec3 normal;
			const Sphere* sphere;
		};

		RayHitPayload traceRay(const Ray& ray);
		RayHitPayload closestHit(const Ray& ray, float hit_distance, const Sphere* sphere);
		RayHitPayload miss(const Ray& ray);

		Color perPixel(int x, int y);

		const Scene* current_scene = nullptr;
		const Camera* current_camera = nullptr;

		int width;
		int height;

		Color* accumulated_colors = nullptr;
		int accumulated_samples = 1;

		std::vector<int> vertitcal_iter;
	};
}