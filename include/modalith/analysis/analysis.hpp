#pragma once

#include "modalith/core/sequential_tracer.hpp"

#include <cstddef>
#include <vector>

namespace modalith {



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

// ---------------------------------------------------------------------------
// OPD (Optical Path Difference) analysis
// ---------------------------------------------------------------------------

struct OPDSample {
  Vec2 pupil;       // Normalized pupil coordinates (x, y)
  double opd_waves; // OPD in waves (relative to chief ray)
};

struct OPDData {
  std::vector<OPDSample> samples;
  double rms_opd_waves{};       // RMS OPD in waves
  double pv_opd_waves{};        // Peak-to-Valley OPD in waves
  double reference_wavelength_nm{};
};

// ---------------------------------------------------------------------------
// Seidel aberration coefficients
// ---------------------------------------------------------------------------

struct SeidelSurfaceContribution {
  std::size_t surface_index{};
  double S1{};  // Spherical aberration contribution
  double S2{};  // Coma contribution
  double S3{};  // Astigmatism contribution
  double S4{};  // Petzval field curvature contribution
  double S5{};  // Distortion contribution
};

struct SeidelCoefficients {
  double S1{};  // Spherical aberration (W040)
  double S2{};  // Coma (W131)
  double S3{};  // Astigmatism (W222)
  double S4{};  // Petzval field curvature (W220P)
  double S5{};  // Distortion (W311)
  std::vector<SeidelSurfaceContribution> surface_contributions;
};

// ---------------------------------------------------------------------------
// Field curvature and distortion
// ---------------------------------------------------------------------------

struct FieldCurvatureSample {
  double field_angle_deg{};
  double tangential_focus_shift_mm{};
  double sagittal_focus_shift_mm{};
};

struct FieldCurvatureData {
  std::vector<FieldCurvatureSample> samples;
};

struct DistortionSample {
  double field_angle_deg{};
  double distortion_percent{};
  double ideal_height_mm{};
  double actual_height_mm{};
};

struct DistortionData {
  std::vector<DistortionSample> samples;
};

// ---------------------------------------------------------------------------
// Chromatic aberration (lateral and longitudinal)
// ---------------------------------------------------------------------------

struct ChromaticAberrationData {
  double lateral_color_mm{};       // Image height difference (F line - C line)
  double longitudinal_color_mm{};  // Focus shift (F line - C line)
  double reference_wavelength_nm{};
};

// ---------------------------------------------------------------------------
// Encircled energy
// ---------------------------------------------------------------------------

struct EncircledEnergySample {
  double radius_mm{};
  double fraction{};  // 0–1
};

struct EncircledEnergyData {
  std::vector<EncircledEnergySample> samples;
};

// ---------------------------------------------------------------------------
// SequentialAnalysis
// ---------------------------------------------------------------------------

class SequentialAnalysis {
public:
  explicit SequentialAnalysis(const SequentialTracer& tracer) : tracer_(tracer) {}

  // Uniform-area concentric pupil sampling for a specific field and wavelengths.
  [[nodiscard]] SpotDiagram spot_diagram(
      const OpticalSystem& system,
      FieldAngle field,
      const std::vector<double>& wavelengths_nm,
      std::size_t pupil_rings = 10) const;

  // Overload: Compute spot diagram for all fields and wavelengths defined in the system.
  [[nodiscard]] std::vector<SpotDiagram> spot_diagram_all(
      const OpticalSystem& system,
      std::size_t pupil_rings = 10) const;

  // Zemax equivalent: Ray Fan, transverse aberration at the image surface.
  [[nodiscard]] RayFan ray_fan(const OpticalSystem& system,
                              FieldAngle field,
                              double wavelength_nm,
                              std::size_t sample_count = 41) const;

  // Overload: Compute ray fan for all fields and wavelengths in the system.
  [[nodiscard]] std::vector<RayFan> ray_fan_all(
      const OpticalSystem& system,
      std::size_t sample_count = 41) const;

  // OPD (Optical Path Difference) map over a square pupil grid.
  // OPD is computed relative to the chief ray optical path length.
  [[nodiscard]] OPDData opd_map(const OpticalSystem& system, FieldAngle field,
                                double wavelength_nm,
                                std::size_t pupil_grid = 32) const;

  // Third-order (Seidel) aberration coefficients computed from paraxial
  // marginal and chief ray data at each refracting surface.
  [[nodiscard]] SeidelCoefficients seidel_coefficients(
      const OpticalSystem& system, double wavelength_nm) const;

  // Field curvature (tangential and sagittal focus shift) at the specified
  // field angles.
  [[nodiscard]] FieldCurvatureData field_curvature(
      const OpticalSystem& system,
      const std::vector<double>& field_angles_deg,
      double wavelength_nm) const;

  // Distortion (percent) at the specified field angles.
  [[nodiscard]] DistortionData distortion(
      const OpticalSystem& system,
      const std::vector<double>& field_angles_deg,
      double wavelength_nm) const;

  // Lateral and longitudinal chromatic aberration between two wavelengths.
  [[nodiscard]] ChromaticAberrationData chromatic_aberration(
      const OpticalSystem& system, FieldAngle field,
      double wavelength_short_nm = 486.1327,   // F line
      double wavelength_long_nm  = 656.2725,   // C line
      double wavelength_ref_nm   = 587.5618) const;  // d line

  // Strehl ratio via Maréchal approximation: exp(-(2π σ_OPD)²).
  // Accurate for σ < 0.1 waves.
  [[nodiscard]] double strehl_ratio(const OpticalSystem& system, FieldAngle field,
                                    double wavelength_nm,
                                    std::size_t pupil_grid = 32) const;

  // Encircled energy as a function of radius from the centroid.
  [[nodiscard]] EncircledEnergyData encircled_energy(
      const OpticalSystem& system, FieldAngle field,
      const std::vector<double>& wavelengths_nm,
      std::size_t pupil_rings = 10,
      std::size_t radial_bins = 100) const;

private:
  [[nodiscard]] Ray launch_ray(const OpticalSystem& system, FieldAngle field,
                               Vec2 pupil, double wavelength_nm,
                               double entrance_pupil_radius, double entrance_pupil_z) const;
  const SequentialTracer& tracer_;
};

}  // namespace modalith

