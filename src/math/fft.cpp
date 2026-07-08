#include "modalith/math/fft.hpp"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <algorithm>

namespace modalith::math {

namespace {

bool is_power_of_two(std::size_t n) {
  return n > 0 && (n & (n - 1)) == 0;
}

void bit_reverse_copy(std::vector<std::complex<double>>& data) {
  std::size_t n = data.size();
  std::size_t j = 0;
  for (std::size_t i = 0; i < n - 1; ++i) {
    if (i < j) {
      std::swap(data[i], data[j]);
    }
    std::size_t k = n >> 1;
    while (k <= j) {
      j -= k;
      k >>= 1;
    }
    j += k;
  }
}

} // namespace

void fft_1d(std::vector<std::complex<double>>& data, bool inverse) {
  std::size_t n = data.size();
  if (n <= 1) return;
  if (!is_power_of_two(n)) {
    throw std::invalid_argument("FFT size must be a power of 2");
  }

  bit_reverse_copy(data);

  const double pi = std::numbers::pi;
  const double sign = inverse ? 1.0 : -1.0;

  for (std::size_t len = 2; len <= n; len <<= 1) {
    double angle = 2.0 * pi / len * sign;
    std::complex<double> wlen(std::cos(angle), std::sin(angle));
    for (std::size_t i = 0; i < n; i += len) {
      std::complex<double> w(1.0, 0.0);
      for (std::size_t j = 0; j < len / 2; ++j) {
        std::complex<double> u = data[i + j];
        std::complex<double> v = data[i + j + len / 2] * w;
        data[i + j] = u + v;
        data[i + j + len / 2] = u - v;
        w *= wlen;
      }
    }
  }

  if (inverse) {
    for (auto& x : data) {
      x /= static_cast<double>(n);
    }
  }
}

void fft_2d(std::vector<std::complex<double>>& data, std::size_t width, std::size_t height, bool inverse) {
  if (data.size() != width * height) {
    throw std::invalid_argument("FFT 2D data size mismatch");
  }

  // Transform rows
  std::vector<std::complex<double>> row(width);
  for (std::size_t y = 0; y < height; ++y) {
    for (std::size_t x = 0; x < width; ++x) {
      row[x] = data[y * width + x];
    }
    fft_1d(row, inverse);
    for (std::size_t x = 0; x < width; ++x) {
      data[y * width + x] = row[x];
    }
  }

  // Transform columns
  std::vector<std::complex<double>> col(height);
  for (std::size_t x = 0; x < width; ++x) {
    for (std::size_t y = 0; y < height; ++y) {
      col[y] = data[y * width + x];
    }
    fft_1d(col, inverse);
    for (std::size_t y = 0; y < height; ++y) {
      data[y * width + x] = col[y];
    }
  }
}

void fft_shift_2d(std::vector<std::complex<double>>& data, std::size_t width, std::size_t height) {
  std::size_t half_w = width / 2;
  std::size_t half_h = height / 2;
  
  for (std::size_t y = 0; y < half_h; ++y) {
    for (std::size_t x = 0; x < half_w; ++x) {
      // Swap Q1 and Q3
      std::swap(data[y * width + x], data[(y + half_h) * width + (x + half_w)]);
      // Swap Q2 and Q4
      std::swap(data[(y + half_h) * width + x], data[y * width + (x + half_w)]);
    }
  }
}

} // namespace modalith::math
