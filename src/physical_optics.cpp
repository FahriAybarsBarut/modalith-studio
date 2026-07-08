#include "modalith/analysis/physical_optics.hpp"
#include "modalith/math/fft.hpp"
#include "modalith/core/paraxial.hpp"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <algorithm>

namespace modalith {

ComplexField PhysicalOptics::compute_pupil_function(
    const OpticalSystem& system, FieldAngle field, double wavelength_nm,
    std::size_t grid_size) const {
  if (grid_size == 0 || (grid_size & (grid_size - 1)) != 0) {
    throw std::invalid_argument("Grid size must be a power of 2");
  }

  ComplexField pupil_field;
  pupil_field.width = grid_size;
  pupil_field.height = grid_size;
  pupil_field.data.resize(grid_size * grid_size, {0.0, 0.0});

  SequentialAnalysis seq_analysis(tracer_);
  OPDData opd = seq_analysis.opd_map(system, field, wavelength_nm, grid_size - 1);
  
  // The OPDData samples are uniformly distributed over the pupil grid.
  // We need to map them to the ComplexField matrix.
  // Pupil coordinates in OPDData are from -1 to 1.
  for (const auto& sample : opd.samples) {
    // Map [-1, 1] to [0, grid_size - 1]
    int px = static_cast<int>(std::round((sample.pupil.x + 1.0) * 0.5 * (grid_size - 1)));
    int py = static_cast<int>(std::round((sample.pupil.y + 1.0) * 0.5 * (grid_size - 1)));
    
    if (px >= 0 && px < static_cast<int>(grid_size) && py >= 0 && py < static_cast<int>(grid_size)) {
      double phase = 2.0 * std::numbers::pi * sample.opd_waves;
      pupil_field.data[py * grid_size + px] = std::complex<double>(std::cos(phase), std::sin(phase));
    }
  }

  // Calculate pixel size for the pupil. The pupil radius is normalized to 1,
  // so the diameter is 2. Therefore, the pixel size is 2 / (grid_size - 1).
  // But wait, the actual physical pupil diameter is needed for PSF scaling.
  ParaxialCalculator paraxial(system, wavelength_nm);
  ParaxialProperties props = paraxial.compute();
  double pupil_diameter = props.entrance_pupil_dia;
  pupil_field.dx = pupil_diameter / (grid_size - 1);
  pupil_field.dy = pupil_field.dx;

  return pupil_field;
}

ComplexField PhysicalOptics::compute_psf(
    const ComplexField& pupil, double wavelength_nm, double efl) const {
  ComplexField psf;
  psf.width = pupil.width;
  psf.height = pupil.height;
  psf.data = pupil.data; // Copy pupil function

  // Apply 2D FFT
  math::fft_2d(psf.data, psf.width, psf.height, false);
  math::fft_shift_2d(psf.data, psf.width, psf.height);

  // Compute intensity and normalize
  double max_intensity = 0.0;
  for (auto& val : psf.data) {
    double intensity = std::norm(val); // norm(a+bi) = a^2 + b^2
    val = std::complex<double>(intensity, 0.0);
    max_intensity = std::max(max_intensity, intensity);
  }

  if (max_intensity > 0.0) {
    for (auto& val : psf.data) {
      val /= max_intensity;
    }
  }

  // Calculate physical size of the PSF pixels.
  // df = 1 / (N * dx)
  // spatial coordinate on image plane = lambda * efl * df
  double df_x = 1.0 / (pupil.width * pupil.dx);
  double df_y = 1.0 / (pupil.height * pupil.dy);
  psf.dx = (wavelength_nm * 1.0e-6) * std::abs(efl) * df_x;
  psf.dy = (wavelength_nm * 1.0e-6) * std::abs(efl) * df_y;

  return psf;
}

MTFData PhysicalOptics::compute_mtf(
    const ComplexField& psf, double wavelength_nm, double efl, double max_frequency) const {
  MTFData mtf;
  
  // Cutoff frequency = 1 / (lambda * F#)
  // F# = efl / pupil_diameter
  // Wait, we don't have pupil diameter here, but we can compute it from psf.dx
  // df = 1 / (N * dx_pupil), psf.dx = lambda * efl / (N * dx_pupil)
  // pupil_diameter = N * dx_pupil = lambda * efl / psf.dx
  double pupil_diameter = (wavelength_nm * 1.0e-6) * std::abs(efl) / psf.dx;
  double f_number = std::abs(efl) / pupil_diameter;
  mtf.cutoff_frequency = 1.0 / ((wavelength_nm * 1.0e-6) * f_number);
  
  if (max_frequency <= 0.0) {
    max_frequency = mtf.cutoff_frequency;
  }

  // To get MTF, we perform inverse FFT on the PSF (which gives the Optical Transfer Function, OTF).
  // The magnitude of OTF is the MTF.
  std::vector<std::complex<double>> otf_data = psf.data;
  // Inverse FFT shift to put zero-frequency back to corners
  math::fft_shift_2d(otf_data, psf.width, psf.height);
  // IFFT
  math::fft_2d(otf_data, psf.width, psf.height, true);
  
  // The zero frequency component should be at (0, 0)
  double zero_freq_mag = std::abs(otf_data[0]);
  if (zero_freq_mag <= 0.0) zero_freq_mag = 1.0;

  // Calculate spatial frequencies for the OTF grid
  // frequency resolution: df = 1 / (N * psf.dx)
  double df = 1.0 / (psf.width * psf.dx);

  std::size_t num_points = static_cast<std::size_t>(std::ceil(max_frequency / df)) + 1;
  num_points = std::min(num_points, psf.width / 2);

  for (std::size_t i = 0; i < num_points; ++i) {
    double freq = i * df;
    mtf.frequencies_cycles_per_mm.push_back(freq);
    
    // Sagittal MTF (along x axis)
    mtf.sagittal_mtf.push_back(std::abs(otf_data[i]) / zero_freq_mag);
    
    // Tangential MTF (along y axis)
    mtf.tangential_mtf.push_back(std::abs(otf_data[i * psf.width]) / zero_freq_mag);
  }

  return mtf;
}

double GaussianBeam::rayleigh_range() const {
  return std::numbers::pi * waist_radius * waist_radius / (wavelength_nm * 1.0e-6);
}

double GaussianBeam::radius(double z) const {
  double z_rel = z - waist_z;
  double z_R = rayleigh_range();
  return waist_radius * std::sqrt(1.0 + (z_rel / z_R) * (z_rel / z_R));
}

double GaussianBeam::radius_of_curvature(double z) const {
  double z_rel = z - waist_z;
  if (std::abs(z_rel) < 1.0e-15) return std::numeric_limits<double>::infinity();
  double z_R = rayleigh_range();
  return z_rel * (1.0 + (z_R / z_rel) * (z_R / z_rel));
}

double GaussianBeam::divergence_angle() const {
  return (wavelength_nm * 1.0e-6) / (std::numbers::pi * waist_radius);
}

std::vector<GaussianBeam> PhysicalOptics::trace_gaussian_beam(
    const OpticalSystem& system, const GaussianBeam& input_beam) const {
  std::vector<GaussianBeam> beams;
  if (system.surfaces.empty()) return beams;

  beams.push_back(input_beam);

  // q-parameter at surface 0
  // q = (z_surf - waist_z) + i * z_R
  double z_R = input_beam.rayleigh_range();
  std::complex<double> q( -input_beam.waist_z, z_R ); // At z=0 (surface 0), z_rel = -waist_z

  double current_n = MaterialCatalog::air()->refractive_index(input_beam.wavelength_nm, system.temperature_c);

  for (std::size_t i = 0; i < system.surfaces.size(); ++i) {
    const auto& surf = system.surfaces[i];
    double R = surf.profile.radius;
    double n_next = MaterialCatalog::air()->refractive_index(input_beam.wavelength_nm, system.temperature_c);
    if (surf.material_after) {
      n_next = surf.material_after->refractive_index(input_beam.wavelength_nm, system.temperature_c);
    }
    
    // Refraction ABCD matrix
    double A = 1.0;
    double B = 0.0;
    double C = 0.0;
    if (std::isfinite(R) && std::abs(R) > 1e-12) {
      C = (current_n - n_next) / (R * n_next);
    }
    double D = current_n / n_next;

    // Apply refraction: q' = (A*q + B) / (C*q + D)
    q = (A * q + B) / (C * q + D);

    // Extract waist_z and waist_radius for the beam inside this new medium
    double z_rel = q.real();
    double z_R_new = q.imag();
    double w0_new = std::sqrt(z_R_new * (input_beam.wavelength_nm * 1.0e-6) / (std::numbers::pi * n_next));
    
    GaussianBeam beam;
    beam.wavelength_nm = input_beam.wavelength_nm;
    beam.waist_radius = w0_new;
    beam.waist_z = -z_rel; // waist is at -z_rel from current surface
    
    beams.push_back(beam);
    
    // Transfer to next surface
    if (i + 1 < system.surfaces.size()) {
      double t = system.surfaces[i+1].transform.translation.z - surf.transform.translation.z;
      q = q + t;
    }
    current_n = n_next;
  }
  
  return beams;
}

} // namespace modalith
