#include "modalith/analysis/tolerance.hpp"
#include <random>
#include <algorithm>
#include <cmath>

namespace modalith {

OpticalSystem ToleranceAnalysis::perturb_system(
    OpticalSystem system, const ToleranceOperand& op, double value) const {
  if (op.surface_index >= system.surfaces.size()) {
    return system;
  }
  
  auto& surf = system.surfaces[op.surface_index];
  
  switch (op.parameter) {
    case ToleranceOperand::Radius:
      surf.profile.radius = value;
      break;
    case ToleranceOperand::Thickness:
      // Note: thickness is the difference between this surface's z and the next surface's z.
      // Changing thickness means shifting all subsequent surfaces.
      if (op.surface_index + 1 < system.surfaces.size()) {
        double current_t = system.surfaces[op.surface_index+1].transform.translation.z - surf.transform.translation.z;
        double diff = value - current_t;
        for (std::size_t i = op.surface_index + 1; i < system.surfaces.size(); ++i) {
          system.surfaces[i].transform.translation.z += diff;
        }
      }
      break;
    case ToleranceOperand::RefractiveIndex:
      // Simple approximation for tolerancing index: we can't easily change the catalog material,
      // but we could introduce an index offset if the system supported it.
      // For now, this is a placeholder. A robust implementation would use a specialized Material wrapper.
      break;
    case ToleranceOperand::DecenterX:
      surf.transform.translation.x += (value - op.nominal_value);
      break;
    case ToleranceOperand::DecenterY:
      surf.transform.translation.y += (value - op.nominal_value);
      break;
    case ToleranceOperand::TiltX:
      // Simplified: in a real system we would apply a rotation matrix to the transform.
      break;
    case ToleranceOperand::TiltY:
      break;
  }
  
  return system;
}

ToleranceStats ToleranceAnalysis::sensitivity(
    const OpticalSystem& nominal_system,
    const std::vector<ToleranceOperand>& operands,
    CriterionFunction criterion) const {
  
  ToleranceStats stats;
  stats.nominal_criterion = criterion(nominal_system);
  
  double rss_sum = 0.0;
  
  for (const auto& op : operands) {
    OpticalSystem sys_min = perturb_system(nominal_system, op, op.min_val);
    double crit_min = criterion(sys_min);
    
    OpticalSystem sys_max = perturb_system(nominal_system, op, op.max_val);
    double crit_max = criterion(sys_max);
    
    double change_min = crit_min - stats.nominal_criterion;
    double change_max = crit_max - stats.nominal_criterion;
    
    // Take the worst case change
    double worst_change = (std::abs(change_min) > std::abs(change_max)) ? change_min : change_max;
    
    SensitivityResult res;
    res.operand = op;
    res.change_in_criterion = worst_change;
    stats.sensitivities.push_back(res);
    
    rss_sum += worst_change * worst_change;
  }
  
  stats.expected_rss_criterion = stats.nominal_criterion + std::sqrt(rss_sum);
  return stats;
}

ToleranceStats ToleranceAnalysis::monte_carlo(
    const OpticalSystem& nominal_system,
    const std::vector<ToleranceOperand>& operands,
    CriterionFunction criterion,
    std::size_t num_trials) const {
  
  ToleranceStats stats = sensitivity(nominal_system, operands, criterion);
  
  std::mt19937 rng(42); // Deterministic seed for reproducible tests
  std::vector<double> results;
  results.reserve(num_trials);
  
  for (std::size_t i = 0; i < num_trials; ++i) {
    OpticalSystem perturbed = nominal_system;
    
    for (const auto& op : operands) {
      double val = op.nominal_value;
      if (op.distribution == ToleranceDistribution::Uniform) {
        std::uniform_real_distribution<double> dist(op.min_val, op.max_val);
        val = dist(rng);
      } else if (op.distribution == ToleranceDistribution::Normal) {
        // Assume 3 sigma covers the min/max range
        double mean = (op.max_val + op.min_val) / 2.0;
        double stddev = (op.max_val - op.min_val) / 6.0;
        std::normal_distribution<double> dist(mean, stddev);
        val = dist(rng);
        // Clamp to limits
        val = std::clamp(val, op.min_val, op.max_val);
      }
      perturbed = perturb_system(perturbed, op, val);
    }
    
    results.push_back(criterion(perturbed));
  }
  
  std::sort(results.begin(), results.end());
  
  double sum = 0.0;
  for (double r : results) sum += r;
  stats.mc_mean = sum / num_trials;
  
  double sq_sum = 0.0;
  for (double r : results) sq_sum += (r - stats.mc_mean) * (r - stats.mc_mean);
  stats.mc_stddev = std::sqrt(sq_sum / num_trials);
  
  std::size_t idx_90 = static_cast<std::size_t>(0.90 * num_trials);
  std::size_t idx_98 = static_cast<std::size_t>(0.98 * num_trials);
  
  stats.mc_90th_percentile = results[std::min(idx_90, num_trials - 1)];
  stats.mc_98th_percentile = results[std::min(idx_98, num_trials - 1)];
  
  return stats;
}

} // namespace modalith
