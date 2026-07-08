#include "modalith/material/agf_parser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace modalith {
namespace {

[[nodiscard]] std::string to_upper(const std::string& s) {
  std::string result = s;
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::toupper(ch)); });
  return result;
}

/// Trim leading and trailing whitespace.
[[nodiscard]] std::string trim(const std::string& s) {
  const auto start = s.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) return {};
  const auto end = s.find_last_not_of(" \t\r\n");
  return s.substr(start, end - start + 1);
}

}  // namespace

std::size_t AgfParser::load(const std::string& file_path) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    throw std::runtime_error("AgfParser: cannot open file: " + file_path);
  }

  std::string line;
  std::string current_name;
  int current_formula = -1;
  std::vector<double> current_coeffs;
  std::size_t count = 0;

  while (std::getline(file, line)) {
    const std::string trimmed = trim(line);
    if (trimmed.empty()) continue;

    // Comment lines start with CC
    if (trimmed.size() >= 2 && trimmed[0] == 'C' && trimmed[1] == 'C') {
      continue;
    }

    std::istringstream iss(trimmed);
    std::string tag;
    iss >> tag;

    if (tag == "NM") {
      // If we have a pending glass from previous NM block, commit it.
      if (!current_name.empty() && current_formula >= 0 && !current_coeffs.empty()) {
        auto mat = create_material(current_name, current_formula, current_coeffs);
        if (mat) {
          glasses_.insert_or_assign(to_upper(current_name), std::move(mat));
          ++count;
        }
      }

      // Parse: NM glass_name formula_number nd vd status
      iss >> current_name >> current_formula;
      current_coeffs.clear();
    } else if (tag == "CD") {
      // Parse dispersion coefficients
      current_coeffs.clear();
      double val{};
      while (iss >> val) {
        current_coeffs.push_back(val);
      }
    }
    // Other lines (TD, OD, LD, IT, etc.) are read but not used for now.
  }

  // Commit the last glass if present.
  if (!current_name.empty() && current_formula >= 0 && !current_coeffs.empty()) {
    auto mat = create_material(current_name, current_formula, current_coeffs);
    if (mat) {
      glasses_.insert_or_assign(to_upper(current_name), std::move(mat));
      ++count;
    }
  }

  return count;
}

std::shared_ptr<const Material>
AgfParser::find(const std::string& glass_name) const {
  const auto it = glasses_.find(to_upper(glass_name));
  if (it == glasses_.end()) {
    throw std::out_of_range("AgfParser: glass not found: " + glass_name);
  }
  return it->second;
}

std::vector<std::string> AgfParser::available_glasses() const {
  std::vector<std::string> names;
  names.reserve(glasses_.size());
  for (const auto& [key, _] : glasses_) {
    names.push_back(key);
  }
  std::sort(names.begin(), names.end());
  return names;
}

std::shared_ptr<Material>
AgfParser::create_material(const std::string& name, int formula,
                           const std::vector<double>& coeffs) const {
  switch (formula) {
    case 1: {
      // Schott — 6 coefficients
      if (coeffs.size() < 6) return nullptr;
      std::array<double, 6> a{};
      std::copy_n(coeffs.begin(), 6, a.begin());
      return std::make_shared<SchottDispersion>(name, a);
    }
    case 2: {
      // Sellmeier1 (3-term) — 6 coefficients (B1 C1 B2 C2 B3 C3)
      if (coeffs.size() < 6) return nullptr;
      std::array<double, 3> b{coeffs[0], coeffs[2], coeffs[4]};
      std::array<double, 3> c{coeffs[1], coeffs[3], coeffs[5]};
      return std::make_shared<Sellmeier3>(name, b, c);
    }
    case 4: {
      // Herzberger — 6 coefficients
      if (coeffs.size() < 6) return nullptr;
      std::array<double, 6> a{};
      std::copy_n(coeffs.begin(), 6, a.begin());
      return std::make_shared<HerzbergerDispersion>(name, a);
    }
    case 5: {
      // Conrady — 3 coefficients
      if (coeffs.size() < 3) return nullptr;
      return std::make_shared<ConradyDispersion>(name, coeffs[0], coeffs[1],
                                                  coeffs[2]);
    }
    case 7: {
      // Cauchy (Handbook of Optics 2) — 3 coefficients
      if (coeffs.size() < 3) return nullptr;
      return std::make_shared<CauchyDispersion>(name, coeffs[0], coeffs[1],
                                                 coeffs[2]);
    }
    default:
      return nullptr;  // Unsupported formula
  }
}

}  // namespace modalith
