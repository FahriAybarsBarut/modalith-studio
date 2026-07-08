#include "modalith/core/sequential_tracer.hpp"

#include <algorithm>
#include <cmath>

namespace modalith {

std::optional<Vec3> refract(const Vec3& incident, const Vec3& geometric_normal,
                            double index_before, double index_after) {
  if (!(index_before > 0.0) || !(index_after > 0.0) ||
      !std::isfinite(index_before) || !std::isfinite(index_after)) {
    return std::nullopt;
  }

  const Vec3 incoming = normalized(incident);
  Vec3 normal = normalized(geometric_normal);
  if (dot(incoming, normal) > 0.0) {
    normal = -normal;
  }
  const double cosine_incident = std::clamp(-dot(incoming, normal), 0.0, 1.0);
  const double eta = index_before / index_after;
  const double radicand = 1.0 - eta * eta *
      (1.0 - cosine_incident * cosine_incident);
  if (radicand < 0.0) {
    return std::nullopt;
  }
  return normalized(eta * incoming +
                    (eta * cosine_incident - std::sqrt(std::max(0.0, radicand))) * normal);
}

TraceResult SequentialTracer::trace(const OpticalSystem& system, const Ray& input) const {
  TraceResult result;
  result.ray = input;
  result.ray.direction = normalized(input.direction);
  result.segments.reserve(system.surfaces.size());

  double current_index = MaterialCatalog::air()->refractive_index(
      input.wavelength_nm, system.temperature_c);

  for (std::size_t index = 0; index < system.surfaces.size(); ++index) {
    const auto& surface = system.surfaces[index];
    const SurfaceIntersection intersection = intersect_surface(result.ray, surface);
    if (!intersection) {
      result.failed_surface = index;
      result.failure = intersection.failure == IntersectionFailure::OutsideAperture
          ? TraceFailure::Vignetted
          : TraceFailure::MissedSurface;
      return result;
    }

    Vec3 outgoing_direction;
    double next_index = current_index;

    if (surface.profile.type == SurfaceType::CoordinateBreak) {
      // CoordinateBreak: ray passes through with only the coordinate
      // transform applied (handled by intersect_surface). No refraction.
      outgoing_direction = result.ray.direction;
    } else if (surface.is_mirror) {
      // Mirror reflection: d_out = d_in - 2 * dot(d_in, n) * n
      const Vec3 d_in = normalized(result.ray.direction);
      Vec3 n = normalized(intersection.normal_global);
      if (dot(d_in, n) > 0.0) {
        n = -n;
      }
      outgoing_direction = normalized(d_in - 2.0 * dot(d_in, n) * n);
    } else {
      // Standard refraction through the surface.
      const auto mat = surface.material_after
          ? surface.material_after : MaterialCatalog::air();
      next_index = mat->refractive_index(
          input.wavelength_nm, system.temperature_c);
      if (!(next_index > 0.0) || !std::isfinite(next_index)) {
        result.failed_surface = index;
        result.failure = TraceFailure::InvalidIndex;
        return result;
      }

      const auto refracted = refract(result.ray.direction,
                                     intersection.normal_global,
                                     current_index, next_index);
      if (!refracted) {
        result.failed_surface = index;
        result.failure = TraceFailure::TotalInternalReflection;
        return result;
      }
      outgoing_direction = *refracted;
    }

    result.ray.optical_path_length += current_index * intersection.distance;
    result.segments.push_back({.surface_index = index,
                               .intercept = intersection.point_global,
                               .direction_after = outgoing_direction,
                               .refractive_index_before = current_index,
                               .refractive_index_after = next_index,
                               .geometric_length = intersection.distance,
                               .optical_path_length = result.ray.optical_path_length});

    result.ray.direction = outgoing_direction;
    result.ray.origin = intersection.point_global + outgoing_direction * kRayOffset;
    current_index = next_index;
  }
  return result;
}

}  // namespace modalith
