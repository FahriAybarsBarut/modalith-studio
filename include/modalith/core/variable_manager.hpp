#pragma once

#include "modalith/core/sequential_tracer.hpp"
#include "modalith/core/optimization.hpp"

#include <vector>

namespace modalith {

/// Manages the mapping between optimization variables and optical system parameters.
class VariableManager {
public:
  /// Extract current values from the system for the given variable definitions.
  [[nodiscard]] std::vector<double> extract_values(
      const OpticalSystem& system,
      const std::vector<OptimizationVariable>& variables) const;

  /// Apply values back to the system.
  void apply_values(OpticalSystem& system,
                    const std::vector<OptimizationVariable>& variables,
                    const std::vector<double>& values) const;

  /// Clamp values to variable bounds.
  [[nodiscard]] std::vector<double> clamp_to_bounds(
      const std::vector<OptimizationVariable>& variables,
      const std::vector<double>& values) const;
};

}  // namespace modalith
