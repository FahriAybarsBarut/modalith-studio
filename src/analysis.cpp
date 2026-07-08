#include "modalith/analysis/analysis.hpp"

#include <Eigen/Core>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

#include "modalith/core/paraxial.hpp"

namespace modalith {
namespace {

[[nodiscard]] double degrees_to_radians(double degrees) noexcept {
  return degrees * std::numbers::pi / 180.0;
}

[[nodiscard]] ParaxialProperties compute_pupil(const OpticalSystem& system, double wavelength_nm) {
  ParaxialCalculator paraxial(system, wavelength_nm);
  return paraxial.compute();
}

[[nodiscard]] Vec2 final_intercept(const TraceResult& trace) {
  const auto& point = trace.segments.back().intercept;
  return {point.x, point.y};
}

}  // namespace

Ray SequentialAnalysis::launch_ray(const OpticalSystem& system, FieldAngle field,
                                   Vec2 pupil, double wavelength_nm,
                                   double entrance_pupil_radius, double entrance_pupil_z) const {
  if (system.surfaces.empty()) {
    throw std::invalid_argument("Cannot launch a ray into an empty system");
  }
  
  const double aperture = entrance_pupil_radius;
  const double ep_z = entrance_pupil_z;

  const auto& first = system.surfaces.front();
  const double local_x = pupil.x * aperture;
  const double local_y = pupil.y * aperture;
  
  // The ray is directed such that it passes through (local_x, local_y) at the Entrance Pupil plane (ep_z).
  // Its angle is determined by the field angle.
  const Vec3 direction{
    std::tan(degrees_to_radians(field.x_degrees)),
    std::tan(degrees_to_radians(field.y_degrees)),
    1.0
  };
  const Vec3 u = normalized(direction);
  
  // Back-project or forward-project to the first surface's tangent plane (z=0 for local coords).
  // Currently, we just start at z_start slightly before the first surface sag.
  const double first_sag = surface_sag(first.profile, local_x, local_y);
  double z_start = std::min(-1.0, (std::isfinite(first_sag) ? first_sag : 0.0) - 1.0);
  if (ep_z < z_start) {
    z_start = ep_z - 1.0;
  }

  // The ray must pass through (local_x, local_y, ep_z).
  // So origin + distance * u = (local_x, local_y, ep_z)
  // At z = z_start, distance_to_ep = (ep_z - z_start) / u.z
  // origin.x + distance_to_ep * u.x = local_x => origin.x = local_x - distance_to_ep * u.x
  const double dist = (ep_z - z_start) / u.z;
  const Vec3 origin_local{
    local_x - dist * u.x,
    local_y - dist * u.y,
    z_start
  };
  return {
    .origin = first.transform.point_to_global(origin_local),
    .direction = u,
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
    const ParaxialProperties pupil_props = compute_pupil(system, wavelength);
    const double ep_radius = pupil_props.entrance_pupil_dia * 0.5;
    const double ep_z = pupil_props.entrance_pupil_z;

    const auto trace_sample = [&](Vec2 pupil) {
      const TraceResult trace = tracer_.trace(system,
          launch_ray(system, field, pupil, wavelength, ep_radius, ep_z));
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
  
  const ParaxialProperties pupil_props = compute_pupil(system, wavelength_nm);
  const double ep_radius = pupil_props.entrance_pupil_dia * 0.5;
  const double ep_z = pupil_props.entrance_pupil_z;

  const TraceResult chief = tracer_.trace(system,
      launch_ray(system, field, {0.0, 0.0}, wavelength_nm, ep_radius, ep_z));
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
        launch_ray(system, field, {0.0, pupil}, wavelength_nm, ep_radius, ep_z));
    const TraceResult sagittal = tracer_.trace(system,
        launch_ray(system, field, {pupil, 0.0}, wavelength_nm, ep_radius, ep_z));
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

std::vector<SpotDiagram> SequentialAnalysis::spot_diagram_all(
    const OpticalSystem& system,
    std::size_t pupil_rings) const {
  std::vector<SpotDiagram> results;
  if (system.fields.empty() || system.wavelengths_nm.empty()) return results;
  
  for (const auto& field : system.fields) {
    results.push_back(spot_diagram(system, field, system.wavelengths_nm, pupil_rings));
  }
  return results;
}

std::vector<RayFan> SequentialAnalysis::ray_fan_all(
    const OpticalSystem& system,
    std::size_t sample_count) const {
  std::vector<RayFan> results;
  if (system.fields.empty() || system.wavelengths_nm.empty()) return results;

  for (const auto& field : system.fields) {
    for (double wl : system.wavelengths_nm) {
      results.push_back(ray_fan(system, field, wl, sample_count));
    }
  }
  return results;
}

OPDData SequentialAnalysis::opd_map(const OpticalSystem& system, FieldAngle field,
                                    double wavelength_nm, std::size_t pupil_grid) const {
  const ParaxialProperties pupil_props = compute_pupil(system, wavelength_nm);
  const double ep_radius = pupil_props.entrance_pupil_dia * 0.5;
  const double ep_z = pupil_props.entrance_pupil_z;

  const TraceResult chief = tracer_.trace(system, launch_ray(system, field, {0.0, 0.0}, wavelength_nm, ep_radius, ep_z));
  if (!chief || chief.segments.empty()) {
    throw std::runtime_error("Chief ray does not reach the image surface");
  }
  const double chief_opl = chief.ray.optical_path_length;
  
  OPDData data;
  data.reference_wavelength_nm = wavelength_nm;
  
  double pv_min = std::numeric_limits<double>::infinity();
  double pv_max = -std::numeric_limits<double>::infinity();
  double rms_sum = 0.0;
  
  for (std::size_t i = 0; i <= pupil_grid; ++i) {
    for (std::size_t j = 0; j <= pupil_grid; ++j) {
      double px = -1.0 + 2.0 * static_cast<double>(i) / pupil_grid;
      double py = -1.0 + 2.0 * static_cast<double>(j) / pupil_grid;
      if (px * px + py * py > 1.0) continue;
      
      const TraceResult trace = tracer_.trace(system, launch_ray(system, field, {px, py}, wavelength_nm, ep_radius, ep_z));
      if (!trace || trace.segments.empty()) continue;
      
      double opd_mm = trace.ray.optical_path_length - chief_opl;
      double opd_waves = opd_mm / (wavelength_nm * 1.0e-6);
      
      data.samples.push_back({{px, py}, opd_waves});
      pv_min = std::min(pv_min, opd_waves);
      pv_max = std::max(pv_max, opd_waves);
      rms_sum += opd_waves * opd_waves;
    }
  }
  if (!data.samples.empty()) {
    data.pv_opd_waves = pv_max - pv_min;
    data.rms_opd_waves = std::sqrt(rms_sum / data.samples.size());
  }
  return data;
}

SeidelCoefficients SequentialAnalysis::seidel_coefficients(
    const OpticalSystem& system, double wavelength_nm) const {
  // Simplistic placeholder, would require paraxial y-n-u trace.
  SeidelCoefficients coeffs;
  return coeffs;
}

FieldCurvatureData SequentialAnalysis::field_curvature(
    const OpticalSystem& system,
    const std::vector<double>& field_angles_deg,
    double wavelength_nm) const {
  FieldCurvatureData data;
  for (double angle : field_angles_deg) {
    data.samples.push_back({angle, 0.0, 0.0});
  }
  return data;
}

DistortionData SequentialAnalysis::distortion(
    const OpticalSystem& system,
    const std::vector<double>& field_angles_deg,
    double wavelength_nm) const {
  DistortionData data;
  for (double angle : field_angles_deg) {
    data.samples.push_back({angle, 0.0, 0.0, 0.0});
  }
  return data;
}

ChromaticAberrationData SequentialAnalysis::chromatic_aberration(
    const OpticalSystem& system, FieldAngle field,
    double wavelength_short_nm, double wavelength_long_nm, double wavelength_ref_nm) const {
  return ChromaticAberrationData();
}

double SequentialAnalysis::strehl_ratio(const OpticalSystem& system, FieldAngle field,
                                        double wavelength_nm, std::size_t pupil_grid) const {
  OPDData opd = opd_map(system, field, wavelength_nm, pupil_grid);
  if (opd.samples.empty()) return 0.0;
  double variance = opd.rms_opd_waves * opd.rms_opd_waves;
  return std::exp(-(4.0 * std::numbers::pi * std::numbers::pi * variance));
}

EncircledEnergyData SequentialAnalysis::encircled_energy(
    const OpticalSystem& system, FieldAngle field,
    const std::vector<double>& wavelengths_nm,
    std::size_t pupil_rings, std::size_t radial_bins) const {
  return EncircledEnergyData();
}

}  // namespace modalith
