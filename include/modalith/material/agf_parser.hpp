#pragma once

#include "modalith/core/glass_catalog.hpp"
#include "modalith/material/material.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace modalith {

/// Parser for Zemax AGF (glass catalog) files.
///
/// Supports dispersion formula numbers:
///   1 = Schott, 2 = Sellmeier1 (3-term), 4 = Herzberger,
///   5 = Conrady, 7 = Cauchy (Handbook of Optics 2)
class AgfParser : public IGlassCatalogImporter {
public:
  /// Load a .agf file and parse all glass entries.
  /// Returns the number of materials successfully parsed.
  [[nodiscard]] std::size_t load(const std::string& file_path) override;

  /// Find a glass by name (case-insensitive).
  [[nodiscard]] std::shared_ptr<const Material>
  find(const std::string& glass_name) const override;

  /// List all loaded glass names.
  [[nodiscard]] std::vector<std::string> available_glasses() const override;

private:
  std::unordered_map<std::string, std::shared_ptr<const Material>> glasses_;

  /// Create the appropriate Material subclass based on the AGF formula number.
  [[nodiscard]] std::shared_ptr<Material>
  create_material(const std::string& name, int formula,
                  const std::vector<double>& coeffs) const;
};

}  // namespace modalith
