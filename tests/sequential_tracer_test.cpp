#include "modalith/core/sequential_tracer.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>

namespace {
modalith::OpticalSurface plane_at(double z, double aperture,
                                std::shared_ptr<const modalith::Material> material) {
  modalith::OpticalSurface surface;
  surface.transform.translation = {0.0, 0.0, z};
  surface.profile.type = modalith::SurfaceType::Plane;
  surface.semi_diameter = aperture;
  surface.material_after = std::move(material);
  return surface;
}
}  // namespace

TEST(SnellLaw, RefractsAirIntoGlass) {
  const double angle = std::numbers::pi / 6.0;
  const modalith::Vec3 incident{std::sin(angle), 0.0, std::cos(angle)};
  const auto transmitted = modalith::refract(incident, {0.0, 0.0, 1.0}, 1.0, 1.5);
  ASSERT_TRUE(transmitted.has_value());
  EXPECT_NEAR(transmitted->x, 1.0 / 3.0, 1.0e-14);
  EXPECT_NEAR(modalith::norm(*transmitted), 1.0, 1.0e-14);
}

TEST(SnellLaw, DetectsTotalInternalReflection) {
  const double angle = std::numbers::pi / 3.0;
  const modalith::Vec3 incident{std::sin(angle), 0.0, std::cos(angle)};
  EXPECT_FALSE(modalith::refract(incident, {0.0, 0.0, 1.0}, 1.5, 1.0));
}

TEST(SequentialTracer, ParallelPlateRestoresIncidentAngle) {
  const auto glass = std::make_shared<modalith::ConstantIndex>("TEST_GLASS", 1.5);
  modalith::OpticalSystem system;
  system.surfaces = {plane_at(0.0, 100.0, glass),
                     plane_at(10.0, 100.0, modalith::MaterialCatalog::air()),
                     plane_at(20.0, 100.0, modalith::MaterialCatalog::air())};
  const modalith::Ray input{.origin = {0.0, 0.0, -1.0},
                          .direction = modalith::normalized({0.2, 0.0, 1.0})};
  const modalith::SequentialTracer tracer;
  const auto result = tracer.trace(system, input);
  ASSERT_TRUE(result);
  ASSERT_EQ(result.segments.size(), 3U);
  EXPECT_NEAR(result.ray.direction.x, input.direction.x, 1.0e-14);
  EXPECT_NEAR(result.ray.direction.z, input.direction.z, 1.0e-14);
  EXPECT_GT(result.ray.optical_path_length, 20.0);
}

TEST(SequentialTracer, PreservesWavelengthDependentIndexAtSurface) {
  modalith::MaterialCatalog catalog;
  modalith::OpticalSystem system;
  system.surfaces = {plane_at(0.0, 10.0, catalog.find("N-BK7"))};
  const modalith::SequentialTracer tracer;
  const modalith::Ray blue{.origin = {0.0, 0.0, -1.0}, .wavelength_nm = 486.1327};
  const modalith::Ray red{.origin = {0.0, 0.0, -1.0}, .wavelength_nm = 656.2725};
  const auto blue_result = tracer.trace(system, blue);
  const auto red_result = tracer.trace(system, red);
  ASSERT_TRUE(blue_result);
  ASSERT_TRUE(red_result);
  EXPECT_GT(blue_result.segments.front().refractive_index_after,
            red_result.segments.front().refractive_index_after);
}
