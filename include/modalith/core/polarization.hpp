#pragma once

#include <array>
#include <complex>
#include <optional>

namespace modalith {

/// Jones vector representing the polarization state of a ray.
/// Components are the complex electric field amplitudes along the
/// s- and p-polarization directions.
struct JonesVector {
  std::complex<double> s{1.0, 0.0};
  std::complex<double> p{0.0, 0.0};
};

/// 2x2 Jones matrix for representing the effect of an optical element
/// on the polarization state.
struct JonesMatrix {
  std::array<std::complex<double>, 4> m{
    {1.0, 0.0},
    {0.0, 1.0}
  };

  /// Apply this matrix to a Jones vector.
  [[nodiscard]] JonesVector apply(const JonesVector& v) const noexcept;
};

/// Optional polarization state that can be attached to a Ray.
/// When std::nullopt, the ray is treated as unpolarized (geometric-only).
using OptionalPolarization = std::optional<JonesVector>;

/// Compute Fresnel reflection and transmission Jones matrices at a
/// dielectric interface.
/// @param n1 Refractive index of the incident medium.
/// @param n2 Refractive index of the transmitted medium.
/// @param cos_theta_i Cosine of the angle of incidence.
/// @return A pair of {reflection, transmission} Jones matrices.
[[nodiscard]] std::pair<JonesMatrix, JonesMatrix>
fresnel_jones_matrices(double n1, double n2, double cos_theta_i);

}  // namespace modalith
