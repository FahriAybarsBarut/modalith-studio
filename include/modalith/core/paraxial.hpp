/// @file paraxial.hpp
/// @brief Paraxial (first-order) optical properties calculator.
///
/// Implements the classical y-n-u (marginal + chief) paraxial ray trace to
/// derive first-order system properties: EFL, BFL, FFL, F/#, NA, entrance/exit
/// pupil location and diameter, and total system length.
///
/// Physics reference — refraction and transfer equations:
///   Refraction:  n'·u' = n·u − y·φ   where φ = (n' − n) / R
///   Transfer:    y'    = y + u'·t

#pragma once

#include "modalith/core/sequential_tracer.hpp"

#include <cstddef>

namespace modalith {

/// @brief Aggregated first-order optical properties of a sequential system.
struct ParaxialProperties {
  double efl{};                ///< Effective focal length (mm).
  double bfl{};                ///< Back focal length (mm) — last surface to rear focus.
  double ffl{};                ///< Front focal length (mm) — first surface to front focus.
  double f_number{};           ///< Working F-number = EFL / (2 · entrance_pupil_diameter).
  double na{};                 ///< Numerical aperture = n · sin(θ) in image space.
  double entrance_pupil_z{};   ///< Axial position of the entrance pupil (mm, from surface 0).
  double entrance_pupil_dia{}; ///< Diameter of the entrance pupil (mm).
  double exit_pupil_z{};       ///< Axial position of the exit pupil (mm, from last surface).
  double exit_pupil_dia{};     ///< Diameter of the exit pupil (mm).
  double total_length{};       ///< Total axial length from first to last surface (mm).
};

/// @brief Computes first-order paraxial properties of a sequential optical system.
///
/// The calculator works entirely in the paraxial (small-angle) domain using
/// the thin-lens y-n-u ray tracing formalism.  It does **not** rely on the
/// full real-ray tracer (SequentialTracer), making it orders-of-magnitude
/// faster for system pre-analysis and optimiser merit-function evaluation.
class ParaxialCalculator {
public:
  /// @brief Construct a calculator for the given system and reference wavelength.
  /// @param system        The sequential optical system description.
  /// @param wavelength_nm Reference wavelength for refractive-index evaluation.
  explicit ParaxialCalculator(const OpticalSystem& system,
                              double wavelength_nm = 587.5618);

  /// @brief Perform the full paraxial analysis and return all first-order data.
  [[nodiscard]] ParaxialProperties compute() const;

private:
  const OpticalSystem& system_;
  double wavelength_nm_;

  /// @brief Internal y-n-u state vector.
  struct YnuState {
    double y;   ///< Ray height at current surface (mm).
    double n;   ///< Refractive index before current surface.
    double u;   ///< Ray angle before current surface (radians, paraxial).
  };

  /// @brief Find the stop surface index (first surface marked `stop`, or 0).
  [[nodiscard]] std::size_t find_stop_index() const;

  /// @brief Evaluate refractive index of the material *after* surface @p idx.
  [[nodiscard]] double index_after(std::size_t idx) const;

  /// @brief Axial thickness between surface @p idx and the next surface.
  [[nodiscard]] double thickness_after(std::size_t idx) const;

  /// @brief Curvature (1/R) for surface @p idx, handling planar (R = ∞) → 0.
  [[nodiscard]] double curvature(std::size_t idx) const;
};

}  // namespace modalith
