#pragma once

#include "modalith/core/sequential_tracer.hpp"

#include <cstddef>
#include <vector>

namespace modalith {

struct FieldAngle {
  double x_degrees{};
  double y_degrees{};
};

struct SpotSample {
  Vec2 pupil;
  Vec2 image;
  double wavelength_nm{};
};

struct SpotDiagram {
  std::vector<SpotSample> samples;
  Vec2 centroid{};
  double rms_radius{};
  double geometric_radius{};
  std::size_t vignetted_rays{};
};

struct RayFanSample {
  double normalized_pupil{};
  double tangential_error{};
  double sagittal_error{};
};

struct RayFan {
  std::vector<RayFanSample> samples;
};

class SequentialAnalysis {
public:
  explicit SequentialAnalysis(const SequentialTracer& tracer) : tracer_(tracer) {}

  // Uniform-area concentric pupil sampling. Runtime is O(W * R^2 * S), where
  // W is wavelengths, R pupil rings and S optical surfaces.
  [[nodiscard]] SpotDiagram spot_diagram(
      const OpticalSystem& system,
      FieldAngle field,
      const std::vector<double>& wavelengths_nm,
      std::size_t pupil_rings = 10) const;

  // Zemax equivalent: Ray Fan, transverse aberration at the image surface.
  [[nodiscard]] RayFan ray_fan(const OpticalSystem& system,
                              FieldAngle field,
                              double wavelength_nm,
                              std::size_t sample_count = 41) const;

private:
  [[nodiscard]] Ray launch_ray(const OpticalSystem& system, FieldAngle field,
                               Vec2 pupil, double wavelength_nm) const;
  const SequentialTracer& tracer_;
};

}  // namespace modalith
