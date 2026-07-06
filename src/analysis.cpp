#include "modalith/analysis/analysis.hpp"

#include <Eigen/Core>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace modalith {
namespace {

[[nodiscard]] double degrees_to_radians(double degrees) noexcept {
  return degrees * std::numbers::pi / 180.0;
}

[[nodiscard]] double entrance_semi_diameter(const OpticalSystem& system) {
  const auto stop = std::find_if(system.surfaces.begin(), system.surfaces.end(),
      [](const OpticalSurface& surface) { return surface.stop; });
  const auto candidate = stop != system.surfaces.end() ? stop :
      std::find_if(system.surfaces.begin(), system.surfaces.end(),
          [](const OpticalSurface& surface) {
            return std::isfinite(surface.semi_diameter);
          });
  if (candidate == system.surfaces.end() || !(candidate->semi_diameter > 0.0) ||
      !std::isfinite(candidate->semi_diameter)) {
    throw std::invalid_argument("Optical system requires a finite entrance aperture");
  }
  return candidate->semi_diameter;
}

[[nodiscard]] Vec2 final_intercept(const TraceResult& trace) {
  const auto& point = trace.segments.back().intercept;
  return {point.x, point.y};
}

}  // namespace

Ray SequentialAnalysis::launch_ray(const OpticalSystem& system, FieldAngle field,
                                   Vec2 pupil, double wavelength_nm) const {
  if (system.surfaces.empty()) {
    throw std::invalid_argument("Cannot launch a ray into an empty system");
  }
  const double aperture = entrance_semi_diameter(system);
  const auto& first = system.surfaces.front();
  const Vec3 origin_local{pupil.x * aperture, pupil.y * aperture, -1.0e-6};
  const Vec3 direction{
    std::tan(degrees_to_radians(field.x_degrees)),
    std::tan(degrees_to_radians(field.y_degrees)),
    1.0
  };
  return {
    .origin = first.transform.point_to_global(origin_local),
    .direction = normalized(direction),
    .wavelength_nm = wavelength_nm
  };
}

SpotDiagram SequentialAnalysis::spot_diagram(
    const OpticalSystem& system, FieldAngle field,
    const std::vector<double>& wavelengths_nm, std::size_t pupil_rings) const {
  if (wavelengths_nm.empty()) {
    throw std::invalid_argument("Spot diagram requires at least one wavelength");
  }
  if (pupil_rings == 0) {
    throw std::invalid_argument("Spot diagram requires at least one pupil ring");
  }

  SpotDiagram diagram;
  for (double wavelength : wavelengths_nm) {
    const auto trace_sample = [&](Vec2 pupil) {
      const TraceResult trace = tracer_.trace(system,
          launch_ray(system, field, pupil, wavelength));
      if (!trace || trace.segments.empty()) {
        ++diagram.vignetted_rays;
        return;
      }
      diagram.samples.push_back({pupil, final_intercept(trace), wavelength});
    };

    trace_sample({0.0, 0.0});
    for (std::size_t ring = 1; ring <= pupil_rings; ++ring) {
      const double normalized_radius = static_cast<double>(ring) /
                                       static_cast<double>(pupil_rings);
      const std::size_t azimuth_count = 6 * ring;
      for (std::size_t azimuth = 0; azimuth < azimuth_count; ++azimuth) {
        const double angle = 2.0 * std::numbers::pi *
            static_cast<double>(azimuth) / static_cast<double>(azimuth_count);
        trace_sample({normalized_radius * std::cos(angle),
                      normalized_radius * std::sin(angle)});
      }
    }
  }

  if (diagram.samples.empty()) {
    return diagram;
  }

  Eigen::Vector2d centroid = Eigen::Vector2d::Zero();
  for (const auto& sample : diagram.samples) {
    centroid += Eigen::Vector2d{sample.image.x, sample.image.y};
  }
  centroid /= static_cast<double>(diagram.samples.size());
  diagram.centroid = {centroid.x(), centroid.y()};

  double squared_radius_sum = 0.0;
  for (const auto& sample : diagram.samples) {
    const double dx = sample.image.x - diagram.centroid.x;
    const double dy = sample.image.y - diagram.centroid.y;
    const double radius = std::hypot(dx, dy);
    squared_radius_sum += radius * radius;
    diagram.geometric_radius = std::max(diagram.geometric_radius, radius);
  }
  diagram.rms_radius = std::sqrt(squared_radius_sum /
                                 static_cast<double>(diagram.samples.size()));
  return diagram;
}

RayFan SequentialAnalysis::ray_fan(const OpticalSystem& system, FieldAngle field,
                                   double wavelength_nm,
                                   std::size_t sample_count) const {
  if (sample_count < 3) {
    throw std::invalid_argument("Ray fan requires at least three samples");
  }
  const TraceResult chief = tracer_.trace(system,
      launch_ray(system, field, {0.0, 0.0}, wavelength_nm));
  if (!chief || chief.segments.empty()) {
    throw std::runtime_error("Chief ray does not reach the image surface");
  }
  const Vec2 reference = final_intercept(chief);

  RayFan fan;
  fan.samples.reserve(sample_count);
  for (std::size_t sample = 0; sample < sample_count; ++sample) {
    const double pupil = -1.0 + 2.0 * static_cast<double>(sample) /
                                  static_cast<double>(sample_count - 1);
    const TraceResult tangential = tracer_.trace(system,
        launch_ray(system, field, {0.0, pupil}, wavelength_nm));
    const TraceResult sagittal = tracer_.trace(system,
        launch_ray(system, field, {pupil, 0.0}, wavelength_nm));
    if (!tangential || !sagittal || tangential.segments.empty() ||
        sagittal.segments.empty()) {
      continue;
    }
    fan.samples.push_back({
      .normalized_pupil = pupil,
      .tangential_error = final_intercept(tangential).y - reference.y,
      .sagittal_error = final_intercept(sagittal).x - reference.x
    });
  }
  return fan;
}

}  // namespace modalith
