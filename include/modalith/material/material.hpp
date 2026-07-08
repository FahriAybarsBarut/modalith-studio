#pragma once

#include <array>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace modalith {

class Material {
public:
  virtual ~Material() = default;
  [[nodiscard]] virtual double refractive_index(double wavelength_nm,
                                                 double temperature_c = 20.0) const = 0;
  [[nodiscard]] virtual std::string_view name() const noexcept = 0;
};

class ConstantIndex final : public Material {
public:
  ConstantIndex(std::string name, double index);
  [[nodiscard]] double refractive_index(double wavelength_nm,
                                         double temperature_c = 20.0) const override;
  [[nodiscard]] std::string_view name() const noexcept override { return name_; }

private:
  std::string name_;
  double index_{};
};

class Sellmeier3 final : public Material {
public:
  // Sellmeier equation (Schott TIE-29):
  // n^2(lambda) = 1 + sum_i B_i lambda^2 / (lambda^2 - C_i), lambda in um.
  Sellmeier3(std::string name, std::array<double, 3> b, std::array<double, 3> c,
             double dn_dt = 0.0);
  [[nodiscard]] double refractive_index(double wavelength_nm,
                                         double temperature_c = 20.0) const override;
  [[nodiscard]] std::string_view name() const noexcept override { return name_; }

private:
  std::string name_;
  std::array<double, 3> b_{};
  std::array<double, 3> c_{};
  double dn_dt_{};
};

/// Schott 6-coefficient dispersion:
/// n²(λ) = a0 + a1·λ² + a2·λ⁻² + a3·λ⁻⁴ + a4·λ⁻⁶ + a5·λ⁻⁸
/// λ in micrometres.
class SchottDispersion final : public Material {
public:
  SchottDispersion(std::string name, std::array<double, 6> coefficients,
                   double dn_dt = 0.0);
  [[nodiscard]] double refractive_index(double wavelength_nm,
                                         double temperature_c = 20.0) const override;
  [[nodiscard]] std::string_view name() const noexcept override { return name_; }

private:
  std::string name_;
  std::array<double, 6> a_{};
  double dn_dt_{};
};

/// Cauchy dispersion: n(λ) = A + B/λ² + C/λ⁴
/// λ in micrometres.
class CauchyDispersion final : public Material {
public:
  CauchyDispersion(std::string name, double a, double b, double c,
                   double dn_dt = 0.0);
  [[nodiscard]] double refractive_index(double wavelength_nm,
                                         double temperature_c = 20.0) const override;
  [[nodiscard]] std::string_view name() const noexcept override { return name_; }

private:
  std::string name_;
  double a_{};
  double b_{};
  double c_{};
  double dn_dt_{};
};

/// Herzberger dispersion:
/// n = A + B·λ² + C·λ⁴ + D/(λ²−0.028) + E/(λ²−0.028)² + F/(λ²−0.028)³
/// λ in micrometres.
class HerzbergerDispersion final : public Material {
public:
  HerzbergerDispersion(std::string name, std::array<double, 6> coefficients,
                       double dn_dt = 0.0);
  [[nodiscard]] double refractive_index(double wavelength_nm,
                                         double temperature_c = 20.0) const override;
  [[nodiscard]] std::string_view name() const noexcept override { return name_; }

private:
  std::string name_;
  std::array<double, 6> a_{};
  double dn_dt_{};
};

/// Conrady dispersion: n = A + B/λ + C/λ^3.5
/// λ in micrometres.
class ConradyDispersion final : public Material {
public:
  ConradyDispersion(std::string name, double a, double b, double c,
                    double dn_dt = 0.0);
  [[nodiscard]] double refractive_index(double wavelength_nm,
                                         double temperature_c = 20.0) const override;
  [[nodiscard]] std::string_view name() const noexcept override { return name_; }

private:
  std::string name_;
  double a_{};
  double b_{};
  double c_{};
  double dn_dt_{};
};

class MaterialCatalog {
public:
  MaterialCatalog();
  void add(std::shared_ptr<const Material> material);
  [[nodiscard]] std::shared_ptr<const Material> find(std::string_view name) const;
  [[nodiscard]] static std::shared_ptr<const Material> air();

private:
  std::unordered_map<std::string, std::shared_ptr<const Material>> materials_;
};

}  // namespace modalith
