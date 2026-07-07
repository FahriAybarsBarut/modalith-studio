#include "modalith/core/surface.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace modalith {
namespace {

[[nodiscard]] bool is_plane(const SurfaceProfile& profile) noexcept {
  return profile.type == SurfaceType::Plane || !std::isfinite(profile.radius) ||
         std::abs(profile.radius) > 1.0e15;
}

[[nodiscard]] double base_conic_sag(const SurfaceProfile& profile, double radial_squared) {
  if (is_plane(profile)) {
    return 0.0;
  }
  const double curvature = 1.0 / profile.radius;
  const double radicand = 1.0 - (1.0 + profile.conic_constant) *
                                   curvature * curvature * radial_squared;
  if (radicand < 0.0) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  const double denominator = 1.0 + std::sqrt(std::max(0.0, radicand));
  return curvature * radial_squared / denominator;
}

[[nodiscard]] double sag_radial_derivative(const SurfaceProfile& profile,
                                           double radius) {
  if (radius <= kGeometryEpsilon) {
    return 0.0;
  }

  double derivative = 0.0;
  if (!is_plane(profile)) {
    const double sag = base_conic_sag(profile, radius * radius);
    const double denominator = profile.radius - (1.0 + profile.conic_constant) * sag;
    if (std::abs(denominator) <= kGeometryEpsilon) {
      return std::numeric_limits<double>::quiet_NaN();
    }
    derivative = radius / denominator;
  }

  if (profile.type == SurfaceType::EvenAsphere) {
    double radial_power = radius * radius * radius;  // r^3 for d(A4*r^4)/dr / 4.
    double order = 4.0;
    for (double coefficient : profile.even_asphere_coefficients) {
      derivative += order * coefficient * radial_power;
      radial_power *= radius * radius;
      order += 2.0;
    }
  }
  return derivative;
}

[[nodiscard]] std::optional<double> nearest_forward_root(double a, double b, double c) {
  if (std::abs(a) <= kGeometryEpsilon) {
    if (std::abs(b) <= kGeometryEpsilon) {
      return std::nullopt;
    }
    const double root = -c / b;
    return root > kRayOffset ? std::optional<double>{root} : std::nullopt;
  }

  double discriminant = b * b - 4.0 * a * c;
  const double scale = b * b + std::abs(4.0 * a * c);
  if (discriminant < 0.0 && discriminant > -64.0 *
      std::numeric_limits<double>::epsilon() * std::max(1.0, scale)) {
    discriminant = 0.0;
  }
  if (discriminant < 0.0) {
    return std::nullopt;
  }

  // Kahan's cancellation-resistant quadratic formulation.
  const double square_root = std::sqrt(discriminant);
  const double q = -0.5 * (b + std::copysign(square_root, b));
  double first = q / a;
  double second = std::abs(q) > kGeometryEpsilon ? c / q
                                                  : -b / (2.0 * a);
  if (first > second) {
    std::swap(first, second);
  }
  if (first > kRayOffset) {
    return first;
  }
  if (second > kRayOffset) {
    return second;
  }
  return std::nullopt;
}

}  // namespace

double surface_sag(const SurfaceProfile& profile, double x, double y) {
  const double radial_squared = x * x + y * y;
  double sag = base_conic_sag(profile, radial_squared);
  if (!std::isfinite(sag)) {
    return sag;
  }

  if (profile.type == SurfaceType::EvenAsphere) {
    // Zemax equivalent: Even Asphere surface; A4, A6, A8, ... terms.
    double radial_power = radial_squared * radial_squared;
    for (double coefficient : profile.even_asphere_coefficients) {
      sag += coefficient * radial_power;
      radial_power *= radial_squared;
    }
  }
  return sag;
}

Vec3 surface_normal_local(const SurfaceProfile& profile, double x, double y) {
  const double radius = std::hypot(x, y);
  const double radial_derivative = sag_radial_derivative(profile, radius);
  if (!std::isfinite(radial_derivative)) {
    return {std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0};
  }
  if (radius <= kGeometryEpsilon) {
    return {0.0, 0.0, 1.0};
  }
  return normalized({-radial_derivative * x / radius,
                     -radial_derivative * y / radius,
                     1.0});
}

SurfaceIntersection intersect_surface(const Ray& ray, const OpticalSurface& surface) {
  const Vec3 origin = surface.transform.point_to_local(ray.origin);
  const Vec3 direction = surface.transform.direction_to_local(ray.direction);

  std::optional<double> distance;
  const auto& profile = surface.profile;
  if (is_plane(profile)) {
    if (std::abs(direction.z) > kGeometryEpsilon) {
      const double plane_distance = -origin.z / direction.z;
      if (plane_distance > kRayOffset) {
        distance = plane_distance;
      }
    }
  } else if (profile.type != SurfaceType::EvenAsphere) {
    // Rotational conic implicit equation:
    // x^2 + y^2 + (1+k)z^2 - 2Rz = 0.
    const double one_plus_k = 1.0 + profile.conic_constant;
    const double a = direction.x * direction.x + direction.y * direction.y +
                     one_plus_k * direction.z * direction.z;
    const double b = 2.0 * (origin.x * direction.x + origin.y * direction.y +
                     one_plus_k * origin.z * direction.z -
                     profile.radius * direction.z);
    const double c = origin.x * origin.x + origin.y * origin.y +
                     one_plus_k * origin.z * origin.z -
                     2.0 * profile.radius * origin.z;
    distance = nearest_forward_root(a, b, c);
  } else {
    if (std::abs(direction.z) > kGeometryEpsilon) {
      double estimate = -origin.z / direction.z;
      estimate = std::max(estimate, kRayOffset * 2.0);
      constexpr std::size_t maximum_iterations = 40;
      constexpr double absolute_tolerance = 1.0e-12;
      bool converged = false;
      for (std::size_t iteration = 0; iteration < maximum_iterations; ++iteration) {
        const Vec3 point = origin + direction * estimate;
        const double sag = surface_sag(profile, point.x, point.y);
        const double radius = std::hypot(point.x, point.y);
        const double dsag_dr = sag_radial_derivative(profile, radius);
        if (!std::isfinite(sag) || !std::isfinite(dsag_dr)) {
          break;
        }
        const double radial_rate = radius > kGeometryEpsilon
            ? (point.x * direction.x + point.y * direction.y) / radius
            : 0.0;
        const double residual = point.z - sag;
        const double jacobian = direction.z - dsag_dr * radial_rate;
        if (std::abs(residual) <= absolute_tolerance) {
          if (estimate > kRayOffset) {
            distance = estimate;
          }
          converged = true;
          break;
        }
        if (std::abs(jacobian) <= kGeometryEpsilon) {
          break;
        }
        const double step = residual / jacobian;
        estimate -= step;
        if (!(estimate > kRayOffset) || !std::isfinite(estimate)) {
          break;
        }
      }

      // Bisection fallback when Newton-Raphson fails to converge.
      if (!converged && !distance) {
        const double planar_estimate = std::abs(origin.z / direction.z);
        double t_lo = kRayOffset;
        double t_hi = std::max(planar_estimate * 3.0, kRayOffset * 4.0);

        auto residual_at = [&](double t) -> double {
          const Vec3 p = origin + direction * t;
          return p.z - surface_sag(profile, p.x, p.y);
        };

        double f_lo = residual_at(t_lo);
        double f_hi = residual_at(t_hi);

        if (std::isfinite(f_lo) && std::isfinite(f_hi) && f_lo * f_hi < 0.0) {
          constexpr std::size_t bisection_iterations = 64;
          for (std::size_t i = 0; i < bisection_iterations; ++i) {
            const double t_mid = 0.5 * (t_lo + t_hi);
            const double f_mid = residual_at(t_mid);
            if (!std::isfinite(f_mid)) {
              break;
            }
            if (std::abs(f_mid) <= absolute_tolerance) {
              if (t_mid > kRayOffset) {
                distance = t_mid;
              }
              break;
            }
            if (f_lo * f_mid < 0.0) {
              t_hi = t_mid;
              f_hi = f_mid;
            } else {
              t_lo = t_mid;
              f_lo = f_mid;
            }
            if ((t_hi - t_lo) < absolute_tolerance) {
              const double t_final = 0.5 * (t_lo + t_hi);
              if (t_final > kRayOffset) {
                distance = t_final;
              }
              break;
            }
          }
        }
      }
    }
  }

  if (!distance) {
    return {.failure = IntersectionFailure::BehindRay};
  }
  const Vec3 point_local = origin + direction * *distance;
  const double radial_squared = point_local.x * point_local.x + point_local.y * point_local.y;
  const double aperture_squared = surface.semi_diameter * surface.semi_diameter;
  // Scale-aware aperture tolerance: adapts to the working unit scale.
  // For mm-scale optics (typical), semi_diameter ~ 1-100, so scale_factor ~ 1.
  // For µm-scale micro-optics, semi_diameter ~ 0.001-1, so a larger relative
  // tolerance is needed to prevent false ray rejection.
  const double scale_factor = std::max(1.0, surface.semi_diameter);
  const double aperture_tolerance = 64.0 * std::numeric_limits<double>::epsilon() *
                                    scale_factor * std::max(1.0, aperture_squared);
  if (radial_squared > aperture_squared + aperture_tolerance) {
    return {.point_local = point_local,
            .point_global = surface.transform.point_to_global(point_local),
            .distance = *distance,
            .failure = IntersectionFailure::OutsideAperture};
  }

  const Vec3 normal_local = surface_normal_local(profile, point_local.x, point_local.y);
  if (!std::isfinite(normal_local.x)) {
    return {.point_local = point_local,
            .point_global = surface.transform.point_to_global(point_local),
            .distance = *distance,
            .failure = IntersectionFailure::InvalidSurface};
  }

  return {
    .point_local = point_local,
    .point_global = surface.transform.point_to_global(point_local),
    .normal_global = normalized(surface.transform.direction_to_global(normal_local)),
    .distance = *distance,
    .failure = IntersectionFailure::None
  };
}

}  // namespace modalith
