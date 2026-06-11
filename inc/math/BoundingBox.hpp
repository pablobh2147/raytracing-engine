#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <type_traits>

#include "math/Vector.hpp"

namespace hzr {

template <typename T>
    requires std::is_arithmetic_v<T>
class BoundingBox {
   private:
    Vector3<T> m_min;
    Vector3<T> m_max;

   public:
    constexpr BoundingBox() noexcept : m_min(0), m_max(0) {}
    constexpr BoundingBox(Vector3<T> min, Vector3<T> max) noexcept { SetBounds(min, max); }

    // ---------------------- Getters ----------------------

    [[nodiscard]] constexpr Vector3<T> Min() const noexcept { return m_min; }
    [[nodiscard]] constexpr Vector3<T> Max() const noexcept { return m_max; }
    [[nodiscard]] constexpr Vector3<T> Center() const noexcept { return (m_min + m_max) / T {2}; }
    [[nodiscard]] constexpr Vector3<T> Centroid() const noexcept { return Center(); }
    [[nodiscard]] constexpr Vector3<T> Size() const noexcept { return m_max - m_min; }
    [[nodiscard]] constexpr Vector3<T> Extents() const noexcept { return (m_max - m_min) / T {2}; }

    [[nodiscard]] constexpr T Volume() const noexcept {
        Vector3<T> d = Size();
        return d.x * d.y * d.z;
    }

    [[nodiscard]] constexpr T SurfaceArea() const noexcept {
        Vector3<T> d = Size();
        return 2 * (d.x * d.y + d.y * d.z + d.z * d.x);
    }

    // ---------------------- Setters ----------------------

    constexpr void SetMin(Vector3<T> min) noexcept { SetBounds(min, m_max); }
    constexpr void SetMax(Vector3<T> max) noexcept { SetBounds(m_min, max); }

    constexpr void SetBounds(Vector3<T> a, Vector3<T> b) noexcept {
        m_min = glm::min(a, b);
        m_max = glm::max(a, b);
    }

    // ---------------------- Intersections ----------------------

    [[nodiscard]] constexpr bool Contains(Vector3<T> point) const noexcept {
        return (
            point.x >= m_min.x && point.x <= m_max.x &&
            point.y >= m_min.y && point.y <= m_max.y &&
            point.z >= m_min.z && point.z <= m_max.z  //
        );
    }

    [[nodiscard]] constexpr bool Contains(const BoundingBox<T>& other) const noexcept {
        return (
            other.m_min.x >= m_min.x && other.m_max.x <= m_max.x &&
            other.m_min.y >= m_min.y && other.m_max.y <= m_max.y &&
            other.m_min.z >= m_min.z && other.m_max.z <= m_max.z  //
        );
    }

    [[nodiscard]] constexpr bool Intersects(const BoundingBox<T>& other) const noexcept {
        return (
            other.m_min.x <= m_max.x && other.m_max.x >= m_min.x &&
            other.m_min.y <= m_max.y && other.m_max.y >= m_min.y &&
            other.m_min.z <= m_max.z && other.m_max.z >= m_min.z  //
        );
    }

    [[nodiscard]] constexpr bool Intersects(const BoundingBox<T>& other, BoundingBox<T>& intersection) const noexcept {
        bool intersects = Intersects(other);

        if (intersects) {
            intersection.m_max = glm::min(m_max, other.m_max);
            intersection.m_min = glm::max(m_min, other.m_min);
        }

        return intersects;
    }

    // ---------------------- Utilities ----------------------

    [[nodiscard]] constexpr bool IsEmpty() const noexcept {
        return m_min.x >= m_max.x || m_min.y >= m_max.y || m_min.z >= m_max.z;
    }

    constexpr void Merge(const BoundingBox<T>& other) noexcept {
        m_min = glm::min(m_min, other.m_min);
        m_max = glm::max(m_max, other.m_max);
    }

    [[nodiscard]] constexpr static BoundingBox<T> Merge(std::span<const BoundingBox<T>> boxes) noexcept {
        if (boxes.empty()) {
            return {};
        }

        Vector3<T> min = boxes[0].m_min;
        Vector3<T> max = boxes[0].m_max;

        for (auto it = boxes.begin() + 1; it < boxes.end(); ++it) {
            min = glm::min(min, it->m_min);
            max = glm::max(max, it->m_max);
        }

        return BoundingBox<T>(min, max);
    }

    template <typename F>
        requires std::floating_point<F>
    [[nodiscard]] constexpr Vector3<F> Lerp(Vector3<F> t) const noexcept {
        Vector3<F> fmin = static_cast<Vector3<F>>(m_min);
        Vector3<F> fsize = static_cast<Vector3<F>>(Size());
        return fmin + (fsize * t);
    }

    template <typename F>
        requires std::floating_point<F>
    [[nodiscard]] constexpr Vector3<F> Offset(Vector3<F> point) const noexcept {
        Vector3<F> fmin = static_cast<Vector3<F>>(m_min);
        Vector3<F> fsize = static_cast<Vector3<F>>(Size());

        Vector3<F> result;
        result.x = (fsize.x > 0) ? (point.x - fmin.x) / fsize.x : F {0};
        result.y = (fsize.y > 0) ? (point.y - fmin.y) / fsize.y : F {0};
        result.z = (fsize.z > 0) ? (point.z - fmin.z) / fsize.z : F {0};
        return result;
    }

    constexpr void Enclose(Vector3<T> point) noexcept {
        m_min = glm::min(m_min, point);
        m_max = glm::max(m_max, point);
    }

    [[nodiscard]] constexpr static BoundingBox<T> Enclose(std::span<const Vector3<T>> points) noexcept {
        if (points.empty()) {
            return {};
        }

        Vector3<T> min = points[0];
        Vector3<T> max = points[0];

        for (auto it = points.begin() + 1; it < points.end(); ++it) {
            min = glm::min(min, *it);
            max = glm::max(max, *it);
        }

        return BoundingBox<T>(min, max);
    }

    constexpr void Expand(Vector3<T> offset) noexcept {
        SetBounds(m_min - offset, m_max + offset);
    }

    constexpr void Scale(T factor, Vector3<T> center) noexcept {
        const Vector3<T> min = center + (m_min - center) * factor;
        const Vector3<T> max = center + (m_max - center) * factor;
        SetBounds(min, max);
    }

    constexpr void Scale(T factor) noexcept {
        Scale(factor, Center());
    }

    constexpr void GetCorners(std::span<Vector3<T>, 8> corners) const noexcept {
        corners[0] = m_min;
        corners[1] = Vector3<T>(m_max.x, m_min.y, m_min.z);
        corners[2] = Vector3<T>(m_min.x, m_max.y, m_min.z);
        corners[3] = Vector3<T>(m_max.x, m_max.y, m_min.z);
        corners[4] = Vector3<T>(m_min.x, m_min.y, m_max.z);
        corners[5] = Vector3<T>(m_max.x, m_min.y, m_max.z);
        corners[6] = Vector3<T>(m_min.x, m_max.y, m_max.z);
        corners[7] = m_max;
    }

    [[nodiscard]] constexpr std::array<Vector3<T>, 8> GetCorners() const noexcept {
        std::array<Vector3<T>, 8> corners;
        GetCorners(corners);
        return corners;
    }

    template <typename U>
        requires std::is_arithmetic_v<U>
    [[nodiscard]] constexpr Vector3<U> ClosestPoint(Vector3<U> point) const noexcept {
        return glm::clamp(point, static_cast<Vector3<U>>(m_min), static_cast<Vector3<U>>(m_max));
    }

    template <typename U>
        requires std::is_arithmetic_v<U>
    [[nodiscard]] constexpr U DistanceSquared(Vector3<U> point) const noexcept {
        Vector3<U> closest = ClosestPoint(point);
        Vector3<U> diff = point - closest;
        return glm::dot(diff, diff);
    }

    template <typename U>
        requires std::is_arithmetic_v<U>
    [[nodiscard]] constexpr U Distance(Vector3<U> point) const noexcept {
        return glm::sqrt(DistanceSquared(point));
    }

    [[nodiscard]] constexpr int MaximumExtent() const noexcept {
        Vector3<T> extent = Size();
        if (extent.x > extent.y && extent.x > extent.z) {
            return 0;
        } else if (extent.y > extent.z) {
            return 1;
        } else {
            return 2;
        }
    }

    // ---------------------- Operators ----------------------

    [[nodiscard]] constexpr BoundingBox<T> operator+(Vector3<T> offset) const noexcept {
        return BoundingBox<T>(m_min + offset, m_max + offset);
    }

    [[nodiscard]] constexpr BoundingBox<T> operator-(Vector3<T> offset) const noexcept {
        return BoundingBox<T>(m_min - offset, m_max - offset);
    }

    constexpr BoundingBox<T>& operator+=(Vector3<T> offset) noexcept {
        m_min += offset;
        m_max += offset;
        return *this;
    }

    constexpr BoundingBox<T>& operator-=(Vector3<T> offset) noexcept {
        m_min -= offset;
        m_max -= offset;
        return *this;
    }

    constexpr BoundingBox<T> operator|(const BoundingBox<T>& other) const noexcept {
        return BoundingBox<T>(glm::min(m_min, other.m_min), glm::max(m_max, other.m_max));
    }

    constexpr BoundingBox<T>& operator|=(const BoundingBox<T>& other) noexcept {
        Merge(other);
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const BoundingBox<T>& other) const noexcept {
        return m_min == other.m_min && m_max == other.m_max;
    }

    [[nodiscard]] constexpr bool operator!=(const BoundingBox<T>& other) const noexcept {
        return !(*this == other);
    }
};

using BoundingBoxf = BoundingBox<float>;
using BoundingBoxd = BoundingBox<double>;
using BoundingBoxi32 = BoundingBox<int32_t>;
using BoundingBoxu32 = BoundingBox<uint32_t>;
using BoundingBoxi64 = BoundingBox<int64_t>;
using BoundingBoxu64 = BoundingBox<uint64_t>;

}  // namespace hzr

template <typename T>
    requires std::is_arithmetic_v<T>
struct std::formatter<hzr::BoundingBox<T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const hzr::BoundingBox<T>& box, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "BoundingBox(min: {}, max: {})", box.Min(), box.Max());
    }
};