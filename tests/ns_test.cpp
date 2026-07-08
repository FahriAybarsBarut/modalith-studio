#include <gtest/gtest.h>
#include "modalith/non_sequential/mc_tracer.hpp"

using namespace modalith::ns;

TEST(NonSequentialTest, BasicTrace) {
  Scene scene;

  // Add a light source
  auto light_entity = scene.create_entity("Light");
  auto& light = scene.registry().emplace<LightSource>(light_entity);
  light.type = LightSource::Directional;
  light.direction = {0.0, 0.0, 1.0};
  light.power = 10.0;
  
  auto& tf_l = scene.registry().get<Transform>(light_entity);
  tf_l.translation = {0.0, 0.0, 0.0};

  // Add a detector
  auto det_entity = scene.create_entity("Detector");
  auto& obj = scene.registry().emplace<OpticalObject>(det_entity);
  obj.profile.type = modalith::SurfaceType::Plane;
  obj.semi_diameter = 10.0;
  obj.is_detector = true;

  auto& det = scene.registry().emplace<DetectorData>(det_entity);
  det.physical_width = 20.0;
  det.physical_height = 20.0;
  det.resolution_x = 100;
  det.resolution_y = 100;
  det.init();

  auto& tf_d = scene.registry().get<Transform>(det_entity);
  tf_d.translation = {0.0, 0.0, 50.0};

  // Run trace
  MCTracer tracer(scene);
  tracer.trace_all(1000, 2);

  // Calculate total power on detector
  double total_power = 0.0;
  for (double p : det.irradiance_map) {
    total_power += p;
  }

  // Directional light shooting straight into a detector covering it
  EXPECT_GT(total_power, 9.0); // should collect ~10W
}
