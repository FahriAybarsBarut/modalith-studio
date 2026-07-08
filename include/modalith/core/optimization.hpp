#pragma once

#include <cstddef>
#include <functional>
#include <limits>
#include <string>
#include <vector>

namespace modalith {

/// Identifies an optimizable variable in the optical system.
struct OptimizationVariable {
  std::size_t surface_index{};
  std::string parameter_name;  // e.g., "radius", "thickness", "conic"
  double current_value{};
  double lower_bound{-std::numeric_limits<double>::infinity()};
  double upper_bound{ std::numeric_limits<double>::infinity()};
};

/// A single operand in the merit function (one residual).
struct MeritOperand {
  std::string type;    // e.g., "EFFL", "SPHA", "DIST"
  double target{};
  double weight{1.0};
  // Evaluation parameters (surface index, field point, wavelength, etc.)
  std::size_t surface{};
  std::size_t field{};
  std::size_t wavelength{};
};

/// The merit function: a weighted sum-of-squares of operand residuals.
struct MeritFunction {
  std::vector<MeritOperand> operands;

  /// Evaluate the total merit value given the current system state.
  /// Implementation will trace rays and compute each operand.
  // [[nodiscard]] double evaluate(const OpticalSystem& system) const;
};

/// Result of a single optimization cycle.
struct OptimizationResult {
  double initial_merit{};
  double final_merit{};
  std::size_t iterations{};
  bool converged{};
};

/// Interface for an optical system optimizer.
class IOptimizer {
 public:
  virtual ~IOptimizer() = default;

  /// Run optimization on the given variables to minimize the merit function.
  // [[nodiscard]] virtual OptimizationResult optimize(
  //     OpticalSystem& system,
  //     const std::vector<OptimizationVariable>& variables,
  //     const MeritFunction& merit) = 0;
};

}  // namespace modalith
