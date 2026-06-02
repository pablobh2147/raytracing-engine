#pragma once

#include <glm/glm.hpp>
#include <vector>

class Camera {

public:
	Camera(float vertical_fov, float near, float far);

	void resize(unsigned int width, unsigned int height);

	const glm::mat4& getProjection() const;
	const glm::mat4& getInverseProjection() const;
	const glm::mat4& getView() const;
	const glm::mat4& getInverseView() const;

	const glm::vec3& getPosition() const;
	const glm::vec3& getDirection() const;

	void setPosition(const glm::vec3& position);
	void setDirection(const glm::vec3& direction);

	void move(const glm::vec3& delta);

	const std::vector<glm::vec3>& getRays() const;

private:
	void recalculateProjection();
	void recalculateView();
	void recalculateRays();

	glm::mat4 projection { 1 };
	glm::mat4 inverse_projection { 1 };
	glm::mat4 view { 1 };
	glm::mat4 inverse_view { 1 };

	float vertical_fov;
	float near;
	float far;

	glm::vec3 position { 0, 0, 0 };
	glm::vec3 direction { 0, 0, 0 };

	std::vector<glm::vec3> rays;

	unsigned int viewport_width = 0;
	unsigned int viewport_height = 0;
};