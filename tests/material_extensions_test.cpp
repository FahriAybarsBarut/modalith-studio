#include "modalith/material/agf_parser.hpp"
#include "modalith/material/material.hpp"

#include <cmath>
#include <gtest/gtest.h>

// ===================================================================
// SchottDispersion tests
// ===================================================================

TEST(SchottDispersion, NbK7MatchesFraunhoferDLine) {
  // N-BK7 Schott coefficients
  const std::array<double, 6> coeffs = {
      2.2718929, -0.010108077, 0.010592509,
      0.00020816965, -7.6472538e-06, 4.9240991e-07};
  modalith::SchottDispersion nbk7("N-BK7", coeffs);

  // d-line = 587.5618 nm, expected n_d ≈ 1.5168
  const double n = nbk7.refractive_index(587.5618);
  EXPECT_NEAR(n, 1.5168, 1.0e-4);
}

TEST(SchottDispersion, NormalVisibleDispersion) {
  const std::array<double, 6> coeffs = {
      2.2718929, -0.010108077, 0.010592509,
      0.00020816965, -7.6472538e-06, 4.9240991e-07};
  modalith::SchottDispersion nbk7("N-BK7", coeffs);

  // Blue (F-line) should have higher index than red (C-line)
  EXPECT_GT(nbk7.refractive_index(486.1327), nbk7.refractive_index(656.2725));
}

TEST(SchottDispersion, ThermalCorrection) {
  const std::array<double, 6> coeffs = {
      2.2718929, -0.010108077, 0.010592509,
      0.00020816965, -7.6472538e-06, 4.9240991e-07};
  modalith::SchottDispersion nbk7("N-BK7", coeffs, 5.0e-6);

  const double n_20 = nbk7.refractive_index(587.5618, 20.0);
  const double n_30 = nbk7.refractive_index(587.5618, 30.0);
  EXPECT_NEAR(n_30 - n_20, 5.0e-5, 1.0e-10);
}

// ===================================================================
// CauchyDispersion tests
// ===================================================================

TEST(CauchyDispersion, KnownCoefficients) {
  // n(λ) = A + B/λ² + C/λ⁴
  // For λ=0.5µm (500 nm): n = 1.5 + 0.004/0.25 + 0.0/0.0625 = 1.516
  modalith::CauchyDispersion glass("TEST-CAUCHY", 1.5, 0.004, 0.0);
  const double n = glass.refractive_index(500.0);
  EXPECT_NEAR(n, 1.516, 1.0e-9);
}

TEST(CauchyDispersion, NormalDispersion) {
  modalith::CauchyDispersion glass("TEST-CAUCHY", 1.5, 0.004, 0.0001);
  // Shorter wavelength should have higher index
  EXPECT_GT(glass.refractive_index(400.0), glass.refractive_index(700.0));
}

TEST(CauchyDispersion, WithCTerm) {
  // n(λ) = 1.5 + 0.004/λ² + 0.0001/λ⁴
  // For λ=0.5µm: n = 1.5 + 0.004/0.25 + 0.0001/0.0625 = 1.5 + 0.016 + 0.0016 = 1.5176
  modalith::CauchyDispersion glass("TEST-CAUCHY-C", 1.5, 0.004, 0.0001);
  EXPECT_NEAR(glass.refractive_index(500.0), 1.5176, 1.0e-9);
}

// ===================================================================
// HerzbergerDispersion tests
// ===================================================================

TEST(HerzbergerDispersion, KnownCoefficients) {
  // A=1.5, B=0.001, C=0, D=0.01, E=0, F=0
  // For λ=0.5µm: l²=0.25, l⁴=0.0625, t=0.25-0.028=0.222
  // n = 1.5 + 0.001*0.25 + 0 + 0.01/0.222 + 0 + 0
  // n = 1.5 + 0.00025 + 0.045045... = 1.545295...
  const std::array<double, 6> coeffs = {1.5, 0.001, 0.0, 0.01, 0.0, 0.0};
  modalith::HerzbergerDispersion glass("TEST-HERZBERGER", coeffs);
  const double n = glass.refractive_index(500.0);

  const double l2 = 0.25;
  const double t = l2 - 0.028;
  const double expected = 1.5 + 0.001 * 0.25 + 0.01 / t;
  EXPECT_NEAR(n, expected, 1.0e-10);
}

TEST(HerzbergerDispersion, NormalDispersion) {
  const std::array<double, 6> coeffs = {1.5, -0.001, 0.0, 0.01, 0.0001, 0.0};
  modalith::HerzbergerDispersion glass("TEST-HERZBERGER", coeffs);
  EXPECT_GT(glass.refractive_index(400.0), glass.refractive_index(700.0));
}

// ===================================================================
// ConradyDispersion tests
// ===================================================================

TEST(ConradyDispersion, KnownCoefficients) {
  // n = A + B/λ + C/λ^3.5
  // For λ=0.5µm: n = 1.5 + 0.004/0.5 + 0.0001/(0.5^3.5)
  // = 1.5 + 0.008 + 0.0001/0.08838834... = 1.508 + 0.001131...
  modalith::ConradyDispersion glass("TEST-CONRADY", 1.5, 0.004, 0.0001);
  const double lam = 0.5;
  const double expected = 1.5 + 0.004 / lam + 0.0001 / std::pow(lam, 3.5);
  EXPECT_NEAR(glass.refractive_index(500.0), expected, 1.0e-10);
}

TEST(ConradyDispersion, NormalDispersion) {
  modalith::ConradyDispersion glass("TEST-CONRADY", 1.5, 0.004, 0.0001);
  EXPECT_GT(glass.refractive_index(400.0), glass.refractive_index(700.0));
}

// ===================================================================
// AgfParser tests
// ===================================================================

TEST(AgfParser, LoadTestCatalog) {
  modalith::AgfParser parser;
  const auto count = parser.load("tests/test_data/test.agf");
  EXPECT_EQ(count, 4u);
}

TEST(AgfParser, AvailableGlasses) {
  modalith::AgfParser parser;
  parser.load("tests/test_data/test.agf");
  const auto glasses = parser.available_glasses();
  EXPECT_EQ(glasses.size(), 4u);
}

TEST(AgfParser, FindCaseInsensitive) {
  modalith::AgfParser parser;
  parser.load("tests/test_data/test.agf");

  // All these should find the same glass
  EXPECT_NO_THROW(parser.find("N-BK7-SCHOTT"));
  EXPECT_NO_THROW(parser.find("n-bk7-schott"));
  EXPECT_NO_THROW(parser.find("N-Bk7-Schott"));
}

TEST(AgfParser, SchottFormulaFromAgf) {
  modalith::AgfParser parser;
  parser.load("tests/test_data/test.agf");

  const auto glass = parser.find("N-BK7-SCHOTT");
  // d-line check
  EXPECT_NEAR(glass->refractive_index(587.5618), 1.5168, 1.0e-4);
}

TEST(AgfParser, SellmeierFormulaFromAgf) {
  modalith::AgfParser parser;
  parser.load("tests/test_data/test.agf");

  const auto glass = parser.find("N-BK7-SELLMEIER");
  // Should match the built-in N-BK7 Sellmeier value
  EXPECT_NEAR(glass->refractive_index(587.5618), 1.5168, 1.0e-4);
}

TEST(AgfParser, CauchyFormulaFromAgf) {
  modalith::AgfParser parser;
  parser.load("tests/test_data/test.agf");

  const auto glass = parser.find("TEST-CAUCHY");
  // n = 1.5 + 0.004/λ² with λ=0.5876µm
  const double lam = 0.5876;
  const double expected = 1.5 + 0.004 / (lam * lam);
  EXPECT_NEAR(glass->refractive_index(587.6), expected, 1.0e-4);
}

TEST(AgfParser, ConradyFormulaFromAgf) {
  modalith::AgfParser parser;
  parser.load("tests/test_data/test.agf");

  const auto glass = parser.find("TEST-CONRADY");
  EXPECT_GT(glass->refractive_index(400.0), 1.0);
}

TEST(AgfParser, ThrowsOnUnknownGlass) {
  modalith::AgfParser parser;
  parser.load("tests/test_data/test.agf");
  EXPECT_THROW(parser.find("DOES-NOT-EXIST"), std::out_of_range);
}

TEST(AgfParser, ThrowsOnMissingFile) {
  modalith::AgfParser parser;
  EXPECT_THROW(parser.load("nonexistent.agf"), std::runtime_error);
}
