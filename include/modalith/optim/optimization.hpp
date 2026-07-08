#pragma once

#include "modalith/core/sequential_tracer.hpp"
#include <vector>
#include <functional>
#include <Eigen/Dense>

namespace modalith {

enum class VariableType {
  Radius,
  Thickness, // Z-translation
  Conic,
  AsphereCoefficient
};

struct Variable {
  std::size_t surface_index;
  VariableType type;
  std::size_t extra_index{0}; // For Asphere coefficients
  double min_val{-1.0e9};
  double max_val{1.0e9};
};

class VariableManager {
public:
  void add_variable(Variable var);
  Eigen::VectorXd get_values(const OpticalSystem& system) const;
  void set_values(OpticalSystem& system, const Eigen::VectorXd& values) const;
  std::size_t count() const { return variables_.size(); }
private:
  std::vector<Variable> variables_;
};

struct Operand {
  double target;
  double weight;
  std::function<double(const OpticalSystem&)> evaluator;
};

class MeritFunction {
public:
  void add_operand(Operand op);
  Eigen::VectorXd evaluate_operands(const OpticalSystem& system) const;
  double evaluate(const OpticalSystem& system) const;
  std::size_t count() const { return operands_.size(); }
private:
  std::vector<Operand> operands_;
};

class LevenbergMarquardt {
public:
  LevenbergMarquardt(const VariableManager& var_mgr, const MeritFunction& merit_fn);
  
  // Optimize returns the optimized system
  OpticalSystem optimize(const OpticalSystem& initial_system, 
                         std::size_t max_iters = 50, 
                         double target_merit = 1e-6);

private:
  Eigen::MatrixXd compute_jacobian(const OpticalSystem& system, const Eigen::VectorXd& x);
  
  const VariableManager& var_mgr_;
  const MeritFunction& merit_fn_;
};

} // namespace modalith
