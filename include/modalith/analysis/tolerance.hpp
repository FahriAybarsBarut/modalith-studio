#pragma once

#include "modalith/core/sequential_tracer.hpp"
#include <string>
#include <vector>
#include <functional>

namespace modalith {

enum class ToleranceDistribution {
  Uniform,
  Normal
};

struct ToleranceOperand {
  std::string name;
  std::size_t surface_index{};
  
  // What to perturb
  enum Parameter {
    Radius,
    Thickness,
    RefractiveIndex,
    DecenterX,
    DecenterY,
    TiltX,
    TiltY
  } parameter{Thickness};

  double nominal_value{};
  double min_val{};
  double max_val{};
  
  ToleranceDistribution distribution{ToleranceDistribution::Uniform};
};

struct SensitivityResult {
  ToleranceOperand operand;
  double change_in_criterion{};
};

struct ToleranceStats {
  double nominal_criterion{};
  double expected_rss_criterion{};
  
  std::vector<SensitivityResult> sensitivities;
  
  // Monte Carlo stats
  double mc_mean{};
  double mc_stddev{};
  double mc_90th_percentile{};
  double mc_98th_percentile{};
};

class ToleranceAnalysis {
public:
  using CriterionFunction = std::function<double(const OpticalSystem&)>;

  explicit ToleranceAnalysis(const SequentialTracer& tracer) : tracer_(tracer) {}

  /// @brief Performs a sensitivity analysis (one-at-a-time perturbation)
  [[nodiscard]] ToleranceStats sensitivity(
      const OpticalSystem& nominal_system,
      const std::vector<ToleranceOperand>& operands,
      CriterionFunction criterion) const;

  /// @brief Performs a Monte Carlo tolerance analysis
  [[nodiscard]] ToleranceStats monte_carlo(
      const OpticalSystem& nominal_system,
      const std::vector<ToleranceOperand>& operands,
      CriterionFunction criterion,
      std::size_t num_trials = 100) const;

private:
  const SequentialTracer& tracer_;

  OpticalSystem perturb_system(
      OpticalSystem system, const ToleranceOperand& op, double value) const;
};

} // namespace modalith
