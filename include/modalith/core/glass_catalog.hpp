#pragma once

#include "modalith/material/material.hpp"

#include <memory>
#include <string>
#include <vector>

namespace modalith {

/// Interface for importing glass catalog data (e.g., Zemax .agf files).
class IGlassCatalogImporter {
 public:
  virtual ~IGlassCatalogImporter() = default;

  /// Load a glass catalog from the given file path.
  /// Returns the number of materials successfully parsed.
  [[nodiscard]] virtual std::size_t load(const std::string& file_path) = 0;

  /// Retrieve a material by its catalog name (e.g., "N-BK7").
  [[nodiscard]] virtual std::shared_ptr<const Material>
  find(const std::string& glass_name) const = 0;

  /// List all available glass names in the loaded catalog.
  [[nodiscard]] virtual std::vector<std::string> available_glasses() const = 0;
};

}  // namespace modalith
