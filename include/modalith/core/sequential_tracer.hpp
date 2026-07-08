#pragma once

#include "modalith/core/surface.hpp"

#include <optional>
#include <string>
#include <vector>

namespace modalith {

struct FieldAngle {
  double x_degrees{};
  double y_degrees{};
};

struct OpticalSystem {
  std::string title;
  std::vector<OpticalSurface> surfaces;
  
  std::vector<FieldAngle> fields;
  std::vector<double> wavelengths_nm;
  std::size_t primary_wavelength_index{0};

  double temperature_c{20.0};
};

enum class TraceFailure {
  None,
  MissedSurface,
  Vignetted,
  TotalInternalReflection,
  InvalidIndex
};

struct RaySegment {
  std::size_t surface_index{};
  Vec3 intercept{};
  Vec3 direction_after{};
  double refractive_index_before{1.0};
  double refractive_index_after{1.0};
  double geometric_length{};
  double optical_path_length{};
};

struct TraceResult {
  Ray ray;
  std::vector<RaySegment> segments;
  TraceFailure failure{TraceFailure::None};
  std::size_t failed_surface{};

  [[nodiscard]] explicit operator bool() const noexcept {
    return failure == TraceFailure::None;
  }
};

// Vector Snell law. The returned direction is unit length. A missing value
// denotes total internal reflection (Zemax equivalent: TIR ray error).
[[nodiscard]] std::optional<Vec3> refract(const Vec3& incident,
                                         const Vec3& geometric_normal,
                                         double index_before,
                                         double index_after);

class SequentialTracer {
public:
  [[nodiscard]] TraceResult trace(const OpticalSystem& system, const Ray& input) const;
};

}  // namespace modalith
