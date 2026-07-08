#pragma once

#include "modalith/core/geometry.hpp"
#include "modalith/material/material.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace modalith {

enum class SurfaceType {
  Plane,
  Sphere,
  Conic,
  EvenAsphere,
  OddAsphere,
  ZernikeSag,
  CoordinateBreak,
  Mirror,
  Biconic // Represents Cylindrical, Toroidal via different X and Y radii
};

struct SurfaceProfile {
  SurfaceType type{SurfaceType::Plane};
  double radius{std::numeric_limits<double>::infinity()}; // Base radius (Y-axis)
  double conic_constant{}; // Conic constant (Y-axis)
  
  double radius_x{std::numeric_limits<double>::infinity()}; // Radius for X-axis
  double conic_x{}; // Conic constant for X-axis

  // Coefficients A4, A6, A8, ... in mm^-(order-1).
  std::vector<double> even_asphere_coefficients;

  // OddAsphere: coefficients A1, A2, A3, ... (general polynomial)
  std::vector<double> odd_asphere_coefficients;

  // ZernikeSag: Zernike coefficients (Fringe ordering, starting from Z1)
  std::vector<double> zernike_coefficients;
  double zernike_norm_radius{1.0};  // Normalization radius for Zernike polynomials
};

struct OpticalSurface {
  std::string label;
  Transform transform;
  SurfaceProfile profile;
  double semi_diameter{std::numeric_limits<double>::infinity()};
  std::shared_ptr<const Material> material_after{MaterialCatalog::air()};
  bool stop{};
  bool is_mirror{false};  // If true, ray is reflected instead of refracted
};

enum class IntersectionFailure {
  None,
  BehindRay,
  OutsideAperture,
  NoConvergence,
  InvalidSurface
};

struct SurfaceIntersection {
  Vec3 point_local{};
  Vec3 point_global{};
  Vec3 normal_global{};
  double distance{};
  IntersectionFailure failure{IntersectionFailure::None};

  [[nodiscard]] explicit operator bool() const noexcept {
    return failure == IntersectionFailure::None;
  }
};

[[nodiscard]] double surface_sag(const SurfaceProfile& profile, double x, double y);
[[nodiscard]] Vec3 surface_normal_local(const SurfaceProfile& profile, double x, double y);
[[nodiscard]] SurfaceIntersection intersect_surface(const Ray& ray,
                                                    const OpticalSurface& surface);

}  // namespace modalith
