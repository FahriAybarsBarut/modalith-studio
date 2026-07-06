#pragma once

#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace modalith {

inline constexpr double kGeometryEpsilon = 1.0e-12;
inline constexpr double kRayOffset = 1.0e-9;

struct Vec2 {
  double x{};
  double y{};
};

struct Vec3 {
  double x{};
  double y{};
  double z{};

  [[nodiscard]] constexpr Vec3 operator+(const Vec3& rhs) const noexcept {
    return {x + rhs.x, y + rhs.y, z + rhs.z};
  }
  [[nodiscard]] constexpr Vec3 operator-(const Vec3& rhs) const noexcept {
    return {x - rhs.x, y - rhs.y, z - rhs.z};
  }
  [[nodiscard]] constexpr Vec3 operator-() const noexcept { return {-x, -y, -z}; }
  [[nodiscard]] constexpr Vec3 operator*(double scalar) const noexcept {
    return {x * scalar, y * scalar, z * scalar};
  }
  [[nodiscard]] constexpr Vec3 operator/(double scalar) const {
    return {x / scalar, y / scalar, z / scalar};
  }
  constexpr Vec3& operator+=(const Vec3& rhs) noexcept {
    x += rhs.x; y += rhs.y; z += rhs.z; return *this;
  }
};

[[nodiscard]] constexpr Vec3 operator*(double scalar, const Vec3& value) noexcept {
  return value * scalar;
}

[[nodiscard]] constexpr double dot(const Vec3& lhs, const Vec3& rhs) noexcept {
  return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

[[nodiscard]] constexpr Vec3 cross(const Vec3& lhs, const Vec3& rhs) noexcept {
  return {
    lhs.y * rhs.z - lhs.z * rhs.y,
    lhs.z * rhs.x - lhs.x * rhs.z,
    lhs.x * rhs.y - lhs.y * rhs.x
  };
}

[[nodiscard]] inline double norm(const Vec3& value) noexcept {
  return std::sqrt(dot(value, value));
}

[[nodiscard]] inline Vec3 normalized(const Vec3& value) {
  const double length = norm(value);
  if (!(length > kGeometryEpsilon) || !std::isfinite(length)) {
    throw std::invalid_argument("Cannot normalize a zero or non-finite vector");
  }
  return value / length;
}

// Row-major, orthonormal rotation matrix. This small geometry type keeps the
// performance-critical ray path independent of heap-allocating matrix objects.
struct Mat3 {
  std::array<double, 9> m{1.0, 0.0, 0.0,
                          0.0, 1.0, 0.0,
                          0.0, 0.0, 1.0};

  [[nodiscard]] static Mat3 from_euler_xyz(double rx, double ry, double rz) noexcept {
    const double cx = std::cos(rx), sx = std::sin(rx);
    const double cy = std::cos(ry), sy = std::sin(ry);
    const double cz = std::cos(rz), sz = std::sin(rz);
    return {{
      cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx,
      sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx,
      -sy,     cy * sx,                cy * cx
    }};
  }

  [[nodiscard]] constexpr Vec3 operator*(const Vec3& v) const noexcept {
    return {
      m[0] * v.x + m[1] * v.y + m[2] * v.z,
      m[3] * v.x + m[4] * v.y + m[5] * v.z,
      m[6] * v.x + m[7] * v.y + m[8] * v.z
    };
  }

  [[nodiscard]] constexpr Mat3 transpose() const noexcept {
    return {{m[0], m[3], m[6], m[1], m[4], m[7], m[2], m[5], m[8]}};
  }
};

struct Transform {
  Mat3 rotation{};
  Vec3 translation{};

  [[nodiscard]] Vec3 point_to_local(const Vec3& point) const noexcept {
    return rotation.transpose() * (point - translation);
  }
  [[nodiscard]] Vec3 direction_to_local(const Vec3& direction) const noexcept {
    return rotation.transpose() * direction;
  }
  [[nodiscard]] Vec3 point_to_global(const Vec3& point) const noexcept {
    return rotation * point + translation;
  }
  [[nodiscard]] Vec3 direction_to_global(const Vec3& direction) const noexcept {
    return rotation * direction;
  }
};

struct Ray {
  Vec3 origin{};
  Vec3 direction{0.0, 0.0, 1.0};
  double wavelength_nm{587.5618};
  double intensity{1.0};
  double optical_path_length{};
};

}  // namespace modalith
