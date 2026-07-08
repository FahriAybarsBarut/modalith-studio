#pragma once

#include <complex>
#include <vector>
#include <cstddef>

namespace modalith::math {

/// @brief In-place 1D Fast Fourier Transform (Cooley-Tukey Radix-2)
/// @param data The complex data array. Size must be a power of 2.
/// @param inverse If true, computes the Inverse FFT.
void fft_1d(std::vector<std::complex<double>>& data, bool inverse = false);

/// @brief In-place 2D Fast Fourier Transform
/// @param data The 2D complex data array in row-major order.
/// @param width The width of the grid (must be power of 2).
/// @param height The height of the grid (must be power of 2).
/// @param inverse If true, computes the Inverse 2D FFT.
void fft_2d(std::vector<std::complex<double>>& data, std::size_t width, std::size_t height, bool inverse = false);

/// @brief FFT Shift (moves zero-frequency component to center of array)
/// @param data The 2D complex data array in row-major order.
/// @param width The width of the grid.
/// @param height The height of the grid.
void fft_shift_2d(std::vector<std::complex<double>>& data, std::size_t width, std::size_t height);

} // namespace modalith::math
