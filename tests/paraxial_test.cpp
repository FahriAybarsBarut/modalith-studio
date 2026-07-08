/// @file paraxial_test.cpp
/// @brief Unit tests for ParaxialCalculator.
///
/// Test lens: N-BK7 plano-convex singlet
///   Surface 0:  R₁ = 100 mm  (convex),  material after = N-BK7 (n_d = 1.5168)
///   Surface 1:  R₂ = ∞  (plano),  material after = air
///
/// Expected EFL (lensmaker's equation, thick lens):
///   1/f = (n−1)·[ 1/R₁ − 1/R₂ + (n−1)·t / (n·R₁·R₂) ]
///   With R₂ → ∞:  1/f = (n−1) / R₁ = 0.5168 / 100 = 0.005168
///   f_thin ≈ 193.50 mm
///
///   Thick-lens correction (principal plane shift) gives EFL ≈ 193.7 mm.
///   BFL = EFL − t·(n−1)/(n·R₁) = 193.7 − 10·0.5168/(1.5168·100) ≈ 190.3 mm

#include <gtest/gtest.h>
#include "modalith/core/paraxial.hpp"
#include "modalith/core/units.hpp"
#include "modalith/core/sequential_tracer.hpp"
#include "modalith/material/material.hpp"
#include <memory>
#include <cmath>

TEST(ParaxialCalculatorTest, Nbk7SingletEfl) {
  using namespace modalith;

  // N-BK7 Sellmeier coefficients
  auto nbk7 = std::make_shared<Sellmeier3>(
      "N-BK7",
      std::array<double, 3>{1.03961212, 0.231792344, 1.01046945},
      std::array<double, 3>{0.00600069867, 0.0200179144, 103.560653});

  const double n_d = nbk7->refractive_index(units::kFraunhofer_d);

  OpticalSystem system;
  system.title = "N-BK7 Plano-Convex Singlet";

  OpticalSurface s0;
  s0.profile.type     = SurfaceType::Sphere;
  s0.profile.radius   = 100.0;
  s0.semi_diameter    = 12.5;
  s0.stop             = true;
  s0.material_after   = nbk7;
  s0.transform.translation = {0.0, 0.0, 0.0};

  OpticalSurface s1;
  s1.profile.type     = SurfaceType::Plane;
  s1.semi_diameter    = 12.5;
  s1.material_after   = MaterialCatalog::air();
  s1.transform.translation = {0.0, 0.0, 10.0};

  system.surfaces = {s0, s1};

  ParaxialCalculator calc(system, units::kFraunhofer_d);
  const ParaxialProperties props = calc.compute();

  const double phi1 = (n_d - 1.0) / 100.0;
  const double u1   = -(12.5 * phi1) / n_d;
  const double y1   = 12.5 + u1 * 10.0;
  const double u2   = n_d * u1 / 1.0;
  const double expected_efl = -12.5 / u2;
  const double expected_bfl = -y1 / u2;
  const double expected_fnum = expected_efl / 25.0;

  EXPECT_NEAR(props.efl, expected_efl, 0.01);
  EXPECT_NEAR(props.bfl, expected_bfl, 0.01);
  EXPECT_NEAR(props.f_number, expected_fnum, 0.01);
  EXPECT_NEAR(props.total_length, 10.0, 0.001);
  EXPECT_NEAR(props.entrance_pupil_dia, 25.0, 0.001);
  EXPECT_NEAR(props.na, std::abs(u2), 0.001);
  EXPECT_NEAR(props.efl, 193.5, 1.0);
}

