#pragma once

#include "modalith/analysis/analysis.hpp"
#include <complex>
#include <vector>
#include <cstddef>

namespace modalith {

struct ComplexField {
  std::vector<std::complex<double>> data;
  std::size_t width{};
  std::size_t height{};
  double dx{}; // Pixel size in x (mm or frequency units)
  double dy{}; // Pixel size in y (mm or frequency units)
};

struct MTFData {
  std::vector<double> frequencies_cycles_per_mm;
  std::vector<double> tangential_mtf;
  std::vector<double> sagittal_mtf;
  double cutoff_frequency{};
};

struct GaussianBeam {
  double wavelength_nm{};
  double waist_radius{}; // w0 (mm)
  double waist_z{};      // distance from the reference surface (mm)
  
  // Computed parameters at a specific z
  double radius(double z) const;
  double radius_of_curvature(double z) const;
  double rayleigh_range() const;
  double divergence_angle() const; // half-angle
};

class PhysicalOptics {
public:
  explicit PhysicalOptics(const SequentialTracer& tracer) : tracer_(tracer) {}

  /// @brief Computes the generalized pupil function for a given field and wavelength.
  /// @param system The optical system.
  /// @param field The field angle.
  /// @param wavelength_nm The wavelength.
  /// @param grid_size The resolution of the pupil grid (must be power of 2, e.g. 128, 256).
  /// @return Complex field representing the pupil.
  [[nodiscard]] ComplexField compute_pupil_function(
      const OpticalSystem& system, FieldAngle field, double wavelength_nm,
      std::size_t grid_size = 128) const;

  /// @brief Computes the Point Spread Function (PSF) intensity using Fraunhofer diffraction.
  /// @param pupil The complex pupil function.
  /// @param wavelength_nm The wavelength.
  /// @param efl Effective focal length.
  /// @return A 2D grid of intensities (real-valued, stored in real part of ComplexField).
  [[nodiscard]] ComplexField compute_psf(
      const ComplexField& pupil, double wavelength_nm, double efl) const;

  /// @brief Computes the Modulation Transfer Function (MTF) from the PSF.
  /// @param psf The point spread function intensity.
  /// @param wavelength_nm The wavelength.
  /// @param efl Effective focal length.
  /// @param max_frequency Maximum spatial frequency to return (cycles/mm).
  /// @return MTFData containing tangential and sagittal MTF up to cutoff.
  [[nodiscard]] MTFData compute_mtf(
      const ComplexField& psf, double wavelength_nm, double efl, double max_frequency = 0.0) const;

  /// @brief Traces a Gaussian beam paraxially through the optical system.
  /// @param system The optical system.
  /// @param input_beam The input Gaussian beam parameters at surface 0.
  /// @return A vector of GaussianBeam parameters after each surface.
  [[nodiscard]] std::vector<GaussianBeam> trace_gaussian_beam(
      const OpticalSystem& system, const GaussianBeam& input_beam) const;

private:
  const SequentialTracer& tracer_;
};

} // namespace modalith
