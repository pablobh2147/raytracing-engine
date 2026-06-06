#pragma once

#include <vector>

#include "math/Matrix.hpp"
#include "math/Vector.hpp"

namespace hzr {

class Camera {
   public:
    Camera(float fov, float near, float far);

    void Resize(uint32_t width, uint32_t height);

    const Matrix4f& GetProjection() const { return m_projection; }
    const Matrix4f& GetInverseProjection() const { return m_inverse_projection; }
    const Matrix4f& GetView() const { return m_view; }
    const Matrix4f& GetInverseView() const { return m_inverse_view; }

    const Vector3f& GetPosition() const { return m_position; }
    const Vector3f& GetDirection() const { return m_direction; }

    void SetPosition(const Vector3f& position) {
        m_position = position;
        CalculateView();
        CalculateRays();
    }

    void SetDirection(const Vector3f& direction) {
        m_direction = direction;
        CalculateView();
        CalculateRays();
    }

    void Move(const Vector3f& delta) {
        m_position += delta;
        CalculateView();
        CalculateRays();
    }

    Vector3f GetRayDirection(uint32_t x, uint32_t y) const;

   private:
    void CalculateProjection();
    void CalculateView();
    void CalculateRays();

    Matrix4f m_projection = {1};
    Matrix4f m_inverse_projection = {1};
    Matrix4f m_view = {1};
    Matrix4f m_inverse_view = {1};

    float m_vertical_fov = 0.0F;
    float m_near = 0.0F;
    float m_far = 0.0F;

    Vector3f m_position = {0, 0, 0};
    Vector3f m_direction = {0, 0, 0};

    std::vector<Vector3f> m_rays;

    uint32_t m_viewport_width = 0;
    uint32_t m_viewport_height = 0;
};

}  // namespace hzr
