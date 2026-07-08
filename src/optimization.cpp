#include "modalith/optim/optimization.hpp"
#include <iostream>
#include <algorithm>

namespace modalith {

void VariableManager::add_variable(Variable var) {
  variables_.push_back(var);
}

Eigen::VectorXd VariableManager::get_values(const OpticalSystem& system) const {
  Eigen::VectorXd x(variables_.size());
  for (std::size_t i = 0; i < variables_.size(); ++i) {
    const auto& var = variables_[i];
    const auto& surf = system.surfaces[var.surface_index];
    if (var.type == VariableType::Radius) {
      x[i] = surf.profile.radius;
    } else if (var.type == VariableType::Thickness) {
      x[i] = surf.transform.translation.z;
    } else if (var.type == VariableType::Conic) {
      x[i] = surf.profile.conic_constant;
    } else if (var.type == VariableType::AsphereCoefficient) {
      if (surf.profile.type == SurfaceType::EvenAsphere && var.extra_index < surf.profile.even_asphere_coefficients.size()) {
        x[i] = surf.profile.even_asphere_coefficients[var.extra_index];
      } else {
        x[i] = 0.0;
      }
    }
  }
  return x;
}

void VariableManager::set_values(OpticalSystem& system, const Eigen::VectorXd& values) const {
  for (std::size_t i = 0; i < variables_.size(); ++i) {
    const auto& var = variables_[i];
    auto& surf = system.surfaces[var.surface_index];
    double val = std::max(var.min_val, std::min(var.max_val, values[i]));
    if (var.type == VariableType::Radius) {
      surf.profile.radius = val;
    } else if (var.type == VariableType::Thickness) {
      surf.transform.translation.z = val;
    } else if (var.type == VariableType::Conic) {
      surf.profile.conic_constant = val;
    } else if (var.type == VariableType::AsphereCoefficient) {
      if (surf.profile.type == SurfaceType::EvenAsphere && var.extra_index < surf.profile.even_asphere_coefficients.size()) {
        surf.profile.even_asphere_coefficients[var.extra_index] = val;
      }
    }
  }
}

void MeritFunction::add_operand(Operand op) {
  operands_.push_back(std::move(op));
}

Eigen::VectorXd MeritFunction::evaluate_operands(const OpticalSystem& system) const {
  Eigen::VectorXd f(operands_.size());
  for (std::size_t i = 0; i < operands_.size(); ++i) {
    f[i] = operands_[i].weight * (operands_[i].evaluator(system) - operands_[i].target);
  }
  return f;
}

double MeritFunction::evaluate(const OpticalSystem& system) const {
  Eigen::VectorXd f = evaluate_operands(system);
  return f.squaredNorm();
}

LevenbergMarquardt::LevenbergMarquardt(const VariableManager& var_mgr, const MeritFunction& merit_fn)
  : var_mgr_(var_mgr), merit_fn_(merit_fn) {}

Eigen::MatrixXd LevenbergMarquardt::compute_jacobian(const OpticalSystem& system, const Eigen::VectorXd& x) {
  std::size_t m = merit_fn_.count();
  std::size_t n = var_mgr_.count();
  Eigen::MatrixXd J(m, n);
  
  double h = 1e-5;
  for (std::size_t j = 0; j < n; ++j) {
    OpticalSystem sys_fwd = system;
    Eigen::VectorXd x_fwd = x;
    x_fwd[j] += h;
    var_mgr_.set_values(sys_fwd, x_fwd);
    Eigen::VectorXd f_fwd = merit_fn_.evaluate_operands(sys_fwd);
    
    OpticalSystem sys_bwd = system;
    Eigen::VectorXd x_bwd = x;
    x_bwd[j] -= h;
    var_mgr_.set_values(sys_bwd, x_bwd);
    Eigen::VectorXd f_bwd = merit_fn_.evaluate_operands(sys_bwd);
    
    J.col(j) = (f_fwd - f_bwd) / (2.0 * h);
  }
  return J;
}

OpticalSystem LevenbergMarquardt::optimize(const OpticalSystem& initial_system, 
                                           std::size_t max_iters, 
                                           double target_merit) {
  OpticalSystem sys = initial_system;
  Eigen::VectorXd x = var_mgr_.get_values(sys);
  
  double lambda = 1e-3;
  
  for (std::size_t i = 0; i < max_iters; ++i) {
    Eigen::VectorXd f = merit_fn_.evaluate_operands(sys);
    double cost = f.squaredNorm();
    if (cost < target_merit) break;
    
    Eigen::MatrixXd J = compute_jacobian(sys, x);
    Eigen::MatrixXd H = J.transpose() * J;
    Eigen::MatrixXd I = Eigen::MatrixXd::Identity(H.rows(), H.cols());
    
    Eigen::VectorXd g = J.transpose() * f;
    
    Eigen::MatrixXd H_lm = H + lambda * I; // Levenberg-Marquardt step
    Eigen::VectorXd delta = H_lm.ldlt().solve(-g);
    
    Eigen::VectorXd x_new = x + delta;
    OpticalSystem sys_new = sys;
    var_mgr_.set_values(sys_new, x_new);
    
    double new_cost = merit_fn_.evaluate(sys_new);
    
    if (new_cost < cost) {
      lambda /= 10.0;
      x = x_new;
      sys = sys_new;
    } else {
      lambda *= 10.0;
    }
  }
  return sys;
}

} // namespace modalith
