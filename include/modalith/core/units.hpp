/// @file units.hpp
/// @brief Physical unit conversion utilities for the Modalith optical engine.
///
/// All conversions are constexpr and zero-overhead.  The internal base units
/// are **millimetres** for length and **nanometres** for wavelength.

#pragma once

namespace modalith::units {

// ---------------------------------------------------------------------------
//  Length conversions (base unit: mm)
// ---------------------------------------------------------------------------

/// @brief Convert millimetres → centimetres.
constexpr double mm_to_cm(double v) noexcept { return v * 0.1; }
/// @brief Convert centimetres → millimetres.
constexpr double cm_to_mm(double v) noexcept { return v * 10.0; }
/// @brief Convert millimetres → inches.
constexpr double mm_to_inch(double v) noexcept { return v / 25.4; }
/// @brief Convert inches → millimetres.
constexpr double inch_to_mm(double v) noexcept { return v * 25.4; }
/// @brief Convert millimetres → metres.
constexpr double mm_to_m(double v) noexcept { return v * 0.001; }
/// @brief Convert metres → millimetres.
constexpr double m_to_mm(double v) noexcept { return v * 1000.0; }

// ---------------------------------------------------------------------------
//  Wavelength conversions (base unit: nm)
// ---------------------------------------------------------------------------

/// @brief Convert nanometres → micrometres.
constexpr double nm_to_um(double v) noexcept { return v * 0.001; }
/// @brief Convert micrometres → nanometres.
constexpr double um_to_nm(double v) noexcept { return v * 1000.0; }
/// @brief Convert nanometres → metres.
constexpr double nm_to_m(double v) noexcept { return v * 1.0e-9; }

// ---------------------------------------------------------------------------
//  Angle conversions
// ---------------------------------------------------------------------------

/// @brief π constant used in unit conversions.
inline constexpr double kPi = 3.14159265358979323846;

/// @brief Convert degrees → radians.
constexpr double deg_to_rad(double v) noexcept { return v * kPi / 180.0; }
/// @brief Convert radians → degrees.
constexpr double rad_to_deg(double v) noexcept { return v * 180.0 / kPi; }
/// @brief Convert arc-minutes → radians.
constexpr double arcmin_to_rad(double v) noexcept { return deg_to_rad(v / 60.0); }
/// @brief Convert arc-seconds → radians.
constexpr double arcsec_to_rad(double v) noexcept { return deg_to_rad(v / 3600.0); }

// ---------------------------------------------------------------------------
//  Frequency ⟷ wavelength conversions
// ---------------------------------------------------------------------------

/// @brief Speed of light in vacuum (m/s).
inline constexpr double kSpeedOfLight = 299792458.0;

/// @brief Convert wavelength (nm) → frequency (Hz) using c = λ·f.
constexpr double wavelength_nm_to_freq_hz(double wl_nm) noexcept {
  return kSpeedOfLight / (wl_nm * 1.0e-9);
}

/// @brief Convert frequency (Hz) → wavelength (nm) using λ = c / f.
constexpr double freq_hz_to_wavelength_nm(double f_hz) noexcept {
  return kSpeedOfLight / f_hz * 1.0e9;
}

// ---------------------------------------------------------------------------
//  Standard Fraunhofer spectral lines (nm)
// ---------------------------------------------------------------------------

inline constexpr double kFraunhofer_d = 587.5618;  ///< He-d  (yellow)
inline constexpr double kFraunhofer_C = 656.2725;  ///< H-α   (red)
inline constexpr double kFraunhofer_F = 486.1327;  ///< H-β   (blue)
inline constexpr double kFraunhofer_e = 546.0740;  ///< Hg-e  (green)
inline constexpr double kFraunhofer_g = 435.8343;  ///< Hg-g  (violet)

}  // namespace modalith::units
