#include "modalith/material/material.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

namespace modalith {
namespace {

[[nodiscard]] std::string canonical_name(std::string_view name) {
  std::string result{name};
  std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) {
    return static_cast<char>(std::toupper(ch));
  });
  return result;
}

}  // namespace

ConstantIndex::ConstantIndex(std::string name, double index)
    : name_(std::move(name)), index_(index) {
  if (!(index_ > 0.0) || !std::isfinite(index_)) {
    throw std::invalid_argument("Refractive index must be positive and finite");
  }
}

double ConstantIndex::refractive_index(double wavelength_nm, double temperature_c) const {
  if (!(wavelength_nm > 0.0) || !std::isfinite(wavelength_nm) ||
      !std::isfinite(temperature_c)) {
    throw std::invalid_argument("Wavelength and temperature must be finite");
  }
  return index_;
}

Sellmeier3::Sellmeier3(std::string name, std::array<double, 3> b,
                       std::array<double, 3> c, double dn_dt)
    : name_(std::move(name)), b_(b), c_(c), dn_dt_(dn_dt) {}

double Sellmeier3::refractive_index(double wavelength_nm, double temperature_c) const {
  if (!(wavelength_nm > 0.0) || !std::isfinite(wavelength_nm) ||
      !std::isfinite(temperature_c)) {
    throw std::invalid_argument("Wavelength and temperature must be finite");
  }

  const double wavelength_um = wavelength_nm * 1.0e-3;
  const double lambda_squared = wavelength_um * wavelength_um;
  double index_squared = 1.0;
  for (std::size_t term = 0; term < b_.size(); ++term) {
    const double denominator = lambda_squared - c_[term];
    if (std::abs(denominator) < 1.0e-15) {
      throw std::domain_error("Wavelength lies on a Sellmeier resonance pole");
    }
    index_squared += b_[term] * lambda_squared / denominator;
  }
  if (!(index_squared > 0.0) || !std::isfinite(index_squared)) {
    throw std::domain_error("Sellmeier equation produced a non-physical index");
  }

  // First-order thermal correction. Full Schott six-coefficient thermal
  // dispersion is intentionally reserved for the material-engine milestone.
  return std::sqrt(index_squared) + dn_dt_ * (temperature_c - 20.0);
}

MaterialCatalog::MaterialCatalog() {
  add(air());
  // Schott catalog coefficients; wavelength is in micrometres.
  add(std::make_shared<Sellmeier3>(
      "N-BK7",
      std::array{1.03961212, 0.231792344, 1.01046945},
      std::array{0.00600069867, 0.0200179144, 103.560653},
      3.0e-6));
  add(std::make_shared<Sellmeier3>(
      "N-F2",
      std::array{1.34533359, 0.209073176, 0.937357162},
      std::array{0.00997743871, 0.0470450767, 111.886764},
      2.8e-6));
  add(std::make_shared<Sellmeier3>(
      "FUSED_SILICA",
      std::array{0.6961663, 0.4079426, 0.8974794},
      std::array{0.00467914826, 0.0135120631, 97.9340025},
      10.0e-6));
}

void MaterialCatalog::add(std::shared_ptr<const Material> material) {
  if (!material) {
    throw std::invalid_argument("Cannot add a null material");
  }
  materials_.insert_or_assign(canonical_name(material->name()), std::move(material));
}

std::shared_ptr<const Material> MaterialCatalog::find(std::string_view name) const {
  const auto found = materials_.find(canonical_name(name));
  if (found == materials_.end()) {
    throw std::out_of_range("Material not found: " + std::string{name});
  }
  return found->second;
}

std::shared_ptr<const Material> MaterialCatalog::air() {
  static const auto instance = std::make_shared<const ConstantIndex>("AIR", 1.0);
  return instance;
}

}  // namespace modalith
