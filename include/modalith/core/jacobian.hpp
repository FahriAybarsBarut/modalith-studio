#pragma once

#include <Eigen/Dense>

#include "modalith/core/optimization.hpp"
#include "modalith/core/sequential_tracer.hpp"
#include "modalith/core/variable_manager.hpp"

namespace modalith {

/// Compute the Jacobian matrix of the merit function residuals
/// with respect to the optimization variables using central finite differences.
[[nodiscard]] Eigen::MatrixXd compute_jacobian(
    OpticalSystem& system,
    const SequentialTracer& tracer,
    const MeritFunction& merit,
    const std::vector<OptimizationVariable>& variables,
    const VariableManager& var_mgr,
    double step_size = 1.0e-7);

/// Compute the residual vector for the current system state.
[[nodiscard]] Eigen::VectorXd compute_residuals(
    const OpticalSystem& system,
    const SequentialTracer& tracer,
    const MeritFunction& merit);

}  // namespace modalith
