#include <gtest/gtest.h>
#include <Eigen/Dense>
#include "modalith/core/surface.hpp"
#include "modalith/core/sequential_tracer.hpp"
#include "modalith/core/units.hpp"
#include <cmath>

using namespace modalith;

TEST(SurfaceExtensions, OddAsphereSag) {
  SurfaceProfile profile;
  profile.type = SurfaceType::OddAsphere;
  profile.radius = 100.0; // c = 0.01
  profile.odd_asphere_coefficients = {0.1, 0.02, 0.003}; // A1, A2, A3

  double x = 3.0, y = 4.0; // r = 5.0
  double r = 5.0;
  double c = 0.01;
  double conic_sag = (c * r * r) / (1.0 + std::sqrt(1.0 - c * c * r * r));
  double poly_sag = 0.1 * r + 0.02 * (r * r) + 0.003 * (r * r * r);
  double expected_sag = conic_sag + poly_sag;

  EXPECT_NEAR(surface_sag(profile, x, y), expected_sag, 1.0e-9);
}

TEST(SurfaceExtensions, ZernikeSagDefocus) {
  SurfaceProfile profile;
  profile.type = SurfaceType::ZernikeSag;
  profile.radius = std::numeric_limits<double>::infinity(); // plane base
  profile.zernike_coefficients.resize(4, 0.0);
  profile.zernike_coefficients[3] = 0.5; // Z4
  profile.zernike_norm_radius = 10.0;

  double x = 0.0, y = 10.0;
  EXPECT_NEAR(surface_sag(profile, x, y), 0.5, 1.0e-9);

  x = 0.0; y = 0.0;
  EXPECT_NEAR(surface_sag(profile, x, y), -0.5, 1.0e-9);
}

TEST(SurfaceExtensions, CoordinateBreak) {
  OpticalSurface surface;
  surface.profile.type = SurfaceType::CoordinateBreak;

  Ray ray;
  ray.origin = {0.0, 0.0, -10.0};
  ray.direction = {0.0, 0.0, 1.0};

  auto hit = intersect_surface(ray, surface);
  EXPECT_TRUE(static_cast<bool>(hit));
  EXPECT_NEAR(hit.distance, 0.0, 1.0e-9);
  EXPECT_NEAR(surface_sag(surface.profile, 5.0, 5.0), 0.0, 1.0e-9);
}

TEST(SurfaceExtensions, MirrorReflection) {
  OpticalSystem system;
  OpticalSurface mirror;
  mirror.profile.type = SurfaceType::Plane;
  mirror.is_mirror = true;
  mirror.transform.rotation = modalith::Mat3::from_euler_xyz(units::kPi / 4.0, 0.0, 0.0);
  mirror.transform.translation = {0.0, 0.0, 10.0};
  
  system.surfaces.push_back(mirror);

  SequentialTracer tracer;
  Ray in_ray{{0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}};
  TraceResult result = tracer.trace(system, in_ray); // No wavelength

  ASSERT_EQ(result.segments.size(), 1);
  modalith::Vec3 expected_dir{0.0, 1.0, 0.0};
  
  EXPECT_NEAR(result.segments.back().direction_after.x, expected_dir.x, 1.0e-6);
  EXPECT_NEAR(result.segments.back().direction_after.y, expected_dir.y, 1.0e-6);
  EXPECT_NEAR(result.segments.back().direction_after.z, expected_dir.z, 1.0e-6);
}

TEST(SurfaceExtensions, BiconicCylindrical) {
  SurfaceProfile profile;
  profile.type = SurfaceType::Biconic;
  profile.radius = 10.0; // R_y = 10
  profile.radius_x = std::numeric_limits<double>::infinity(); // Cylinder along Y

  // At x=0, y=5, sag should be same as sphere with R=10
  double sag_y = surface_sag(profile, 0.0, 5.0);
  double expected_sag_y = 10.0 - std::sqrt(100.0 - 25.0);
  EXPECT_NEAR(sag_y, expected_sag_y, 1.0e-9);

  // At x=5, y=0, sag should be 0 (since it's a cylinder along Y)
  // Wait, if it's a cylinder, radius_x = infinity means flat along X!
  // No, if R_x is infinity, the curve in X is flat. So a line varying X has no sag!
  // It's a cylinder with its axis ALONG X! Wait.
  // c_x = 1/R_x = 0. So z = c_y y^2 / ... 
  // It's flat in X, curved in Y. Axis of cylinder is parallel to X-axis.
  double sag_x = surface_sag(profile, 5.0, 0.0);
  EXPECT_NEAR(sag_x, 0.0, 1.0e-9);
}
