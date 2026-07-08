#include <gtest/gtest.h>
#include "modalith/optim/optimization.hpp"
#include <cmath>

using namespace modalith;

TEST(Optimization, VariableManager) {
  OpticalSystem sys;
  sys.surfaces.push_back(OpticalSurface{});
  sys.surfaces[0].profile.radius = 10.0;

  VariableManager mgr;
  mgr.add_variable({0, VariableType::Radius, 0, 1.0, 100.0});

  Eigen::VectorXd x = mgr.get_values(sys);
  EXPECT_EQ(x.size(), 1);
  EXPECT_NEAR(x[0], 10.0, 1e-9);

  x[0] = 20.0;
  mgr.set_values(sys, x);
  EXPECT_NEAR(sys.surfaces[0].profile.radius, 20.0, 1e-9);
}

TEST(Optimization, MeritFunction) {
  OpticalSystem sys;
  sys.surfaces.push_back(OpticalSurface{});
  sys.surfaces[0].profile.radius = 10.0;

  MeritFunction mf;
  // Let's create an operand that just targets radius = 15.0
  mf.add_operand({15.0, 1.0, [](const OpticalSystem& s) { return s.surfaces[0].profile.radius; }});
  
  Eigen::VectorXd f = mf.evaluate_operands(sys);
  EXPECT_EQ(f.size(), 1);
  EXPECT_NEAR(f[0], 10.0 - 15.0, 1e-9);

  double cost = mf.evaluate(sys);
  EXPECT_NEAR(cost, 25.0, 1e-9);
}

TEST(Optimization, LevenbergMarquardt) {
  OpticalSystem sys;
  sys.surfaces.push_back(OpticalSurface{});
  sys.surfaces[0].profile.radius = 10.0;

  VariableManager mgr;
  mgr.add_variable({0, VariableType::Radius, 0, 1.0, 100.0});

  MeritFunction mf;
  // Optimize radius to 15.0
  mf.add_operand({15.0, 1.0, [](const OpticalSystem& s) { return s.surfaces[0].profile.radius; }});

  LevenbergMarquardt lm(mgr, mf);
  OpticalSystem optimized_sys = lm.optimize(sys, 50, 1e-12);

  EXPECT_NEAR(optimized_sys.surfaces[0].profile.radius, 15.0, 1e-4);
}
