#include <gtest/gtest.h>
#include "modalith/analysis/tolerance.hpp"
#include "modalith/analysis/analysis.hpp"

using namespace modalith;

class ToleranceTest : public ::testing::Test {
protected:
  OpticalSystem system;

  void SetUp() override {
    system.title = "Singlet";
    system.wavelengths_nm = { 550.0 };
    system.fields = { {0.0, 0.0} };

    OpticalSurface stop;
    stop.stop = true;
    stop.semi_diameter = 5.0;
    system.surfaces.push_back(stop);

    OpticalSurface s1;
    s1.profile.type = SurfaceType::Sphere;
    s1.profile.radius = 50.0;
    s1.semi_diameter = 6.0;
    s1.transform.translation.z = 10.0;
    MaterialCatalog catalog;
    s1.material_after = catalog.find("N-BK7");
    system.surfaces.push_back(s1);

    OpticalSurface s2;
    s2.profile.radius = std::numeric_limits<double>::infinity();
    s2.semi_diameter = 6.0;
    s2.transform.translation.z = 15.0;
    system.surfaces.push_back(s2);

    OpticalSurface img;
    img.transform.translation.z = 110.0; // rough focus
    system.surfaces.push_back(img);
  }
};

TEST_F(ToleranceTest, SensitivityAnalysis) {
  SequentialTracer tracer;
  ToleranceAnalysis tol(tracer);
  SequentialAnalysis ana(tracer);

  auto criterion = [&](const OpticalSystem& sys) {
    SpotDiagram spot = ana.spot_diagram(sys, sys.fields[0], sys.wavelengths_nm, 3);
    return spot.rms_radius;
  };

  std::vector<ToleranceOperand> ops;
  
  ToleranceOperand op1;
  op1.name = "TRAD_S1";
  op1.surface_index = 1;
  op1.parameter = ToleranceOperand::Radius;
  op1.nominal_value = 50.0;
  op1.min_val = 49.5;
  op1.max_val = 50.5;
  ops.push_back(op1);

  ToleranceOperand op2;
  op2.name = "TTHI_S1";
  op2.surface_index = 1;
  op2.parameter = ToleranceOperand::Thickness;
  op2.nominal_value = 5.0;
  op2.min_val = 4.9;
  op2.max_val = 5.1;
  ops.push_back(op2);

  ToleranceStats stats = tol.sensitivity(system, ops, criterion);

  EXPECT_GT(stats.nominal_criterion, 0.0);
  EXPECT_EQ(stats.sensitivities.size(), 2);
  EXPECT_GT(stats.expected_rss_criterion, stats.nominal_criterion);
}

TEST_F(ToleranceTest, MonteCarloAnalysis) {
  SequentialTracer tracer;
  ToleranceAnalysis tol(tracer);
  SequentialAnalysis ana(tracer);

  auto criterion = [&](const OpticalSystem& sys) {
    SpotDiagram spot = ana.spot_diagram(sys, sys.fields[0], sys.wavelengths_nm, 3);
    return spot.rms_radius;
  };

  std::vector<ToleranceOperand> ops;
  
  ToleranceOperand op1;
  op1.name = "TRAD_S1";
  op1.surface_index = 1;
  op1.parameter = ToleranceOperand::Radius;
  op1.nominal_value = 50.0;
  op1.min_val = 49.5;
  op1.max_val = 50.5;
  op1.distribution = ToleranceDistribution::Normal;
  ops.push_back(op1);

  ToleranceStats stats = tol.monte_carlo(system, ops, criterion, 50);

  EXPECT_GT(stats.mc_mean, 0.0);
  EXPECT_GT(stats.mc_stddev, 0.0);
  EXPECT_GE(stats.mc_90th_percentile, stats.nominal_criterion); // Typically worse or around nominal
}
