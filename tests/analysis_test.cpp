#include "modalith/analysis/analysis.hpp"

#include <gtest/gtest.h>

namespace {
modalith::OpticalSystem unobstructed_system() {
  modalith::OpticalSystem system;
  modalith::OpticalSurface entrance;
  entrance.label = "Stop";
  entrance.semi_diameter = 5.0;
  entrance.stop = true;
  entrance.material_after = modalith::MaterialCatalog::air();
  modalith::OpticalSurface image;
  image.label = "Image";
  image.transform.translation = {0.0, 0.0, 10.0};
  image.semi_diameter = 20.0;
  image.material_after = modalith::MaterialCatalog::air();
  system.surfaces = {entrance, image};
  return system;
}
}  // namespace

TEST(SpotDiagram, UsesDeterministicConcentricPupilSampling) {
  const modalith::SequentialTracer tracer;
  const modalith::SequentialAnalysis analysis{tracer};
  const auto diagram = analysis.spot_diagram(unobstructed_system(), {}, {587.5618}, 3);
  EXPECT_EQ(diagram.samples.size(), 37U);
  EXPECT_EQ(diagram.vignetted_rays, 0U);
  EXPECT_NEAR(diagram.centroid.x, 0.0, 1.0e-13);
  EXPECT_NEAR(diagram.centroid.y, 0.0, 1.0e-13);
  EXPECT_NEAR(diagram.geometric_radius, 5.0, 1.0e-12);
}

TEST(RayFan, SamplesBothPrincipalPupilAxes) {
  const modalith::SequentialTracer tracer;
  const modalith::SequentialAnalysis analysis{tracer};
  const auto fan = analysis.ray_fan(unobstructed_system(), {}, 587.5618, 11);
  ASSERT_EQ(fan.samples.size(), 11U);
  EXPECT_NEAR(fan.samples.front().tangential_error, -5.0, 1.0e-12);
  EXPECT_NEAR(fan.samples.front().sagittal_error, -5.0, 1.0e-12);
  EXPECT_NEAR(fan.samples.back().tangential_error, 5.0, 1.0e-12);
  EXPECT_NEAR(fan.samples.back().sagittal_error, 5.0, 1.0e-12);
}
