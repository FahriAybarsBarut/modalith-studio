#include "modalith/core/surface.hpp"

#include <gtest/gtest.h>

#include <cmath>

TEST(SurfaceSag, SphereMatchesAnalyticSag) {
  modalith::SurfaceProfile sphere{.type = modalith::SurfaceType::Sphere, .radius = 50.0};
  EXPECT_NEAR(modalith::surface_sag(sphere, 5.0, 0.0),
              50.0 - std::sqrt(2475.0), 1.0e-13);
}

TEST(SurfaceSag, ParabolaUsesConicClosedForm) {
  modalith::SurfaceProfile parabola{.type = modalith::SurfaceType::Conic,
                                  .radius = 50.0,
                                  .conic_constant = -1.0};
  EXPECT_NEAR(modalith::surface_sag(parabola, 5.0, 0.0), 0.25, 1.0e-14);
}

TEST(SurfaceSag, EvenAsphereAddsPolynomialTerms) {
  modalith::SurfaceProfile asphere{.type = modalith::SurfaceType::EvenAsphere,
                                 .radius = 50.0,
                                 .conic_constant = -1.0,
                                 .even_asphere_coefficients = {1.0e-5}};
  EXPECT_NEAR(modalith::surface_sag(asphere, 5.0, 0.0), 0.25625, 1.0e-14);
}

TEST(SurfaceIntersection, FindsPlaneInFrontOfRay) {
  modalith::OpticalSurface plane;
  plane.transform.translation = {0.0, 0.0, 10.0};
  plane.profile.type = modalith::SurfaceType::Plane;
  plane.semi_diameter = 5.0;
  const modalith::Ray ray{.origin = {1.0, 0.0, 0.0}};
  const auto hit = modalith::intersect_surface(ray, plane);
  ASSERT_TRUE(hit);
  EXPECT_NEAR(hit.distance, 10.0, 1.0e-12);
}

TEST(SurfaceIntersection, SphereUsesNearestVertexRoot) {
  modalith::OpticalSurface sphere;
  sphere.profile.type = modalith::SurfaceType::Sphere;
  sphere.profile.radius = 50.0;
  sphere.semi_diameter = 10.0;
  const modalith::Ray ray{.origin = {0.0, 0.0, -10.0}};
  const auto hit = modalith::intersect_surface(ray, sphere);
  ASSERT_TRUE(hit);
  EXPECT_NEAR(hit.distance, 10.0, 1.0e-12);
  EXPECT_NEAR(hit.point_global.z, 0.0, 1.0e-12);
}

TEST(SurfaceIntersection, ReportsApertureVignetting) {
  modalith::OpticalSurface plane;
  plane.transform.translation = {0.0, 0.0, 10.0};
  plane.semi_diameter = 1.0;
  const modalith::Ray ray{.origin = {2.0, 0.0, 0.0}};
  const auto hit = modalith::intersect_surface(ray, plane);
  EXPECT_FALSE(hit);
  EXPECT_EQ(hit.failure, modalith::IntersectionFailure::OutsideAperture);
}

TEST(SurfaceIntersection, EvenAsphereGrazingIncidenceUseFallback) {
  // A highly curved even asphere with a grazing-incidence ray to exercise
  // the bisection fallback when Newton-Raphson struggles.
  modalith::OpticalSurface asphere;
  asphere.profile.type = modalith::SurfaceType::EvenAsphere;
  asphere.profile.radius = 10.0;
  asphere.profile.conic_constant = -2.5;
  asphere.profile.even_asphere_coefficients = {5.0e-4, -1.0e-6};
  asphere.semi_diameter = 10.0;
  asphere.transform.translation = {0.0, 0.0, 20.0};
  // Nearly parallel ray (grazing incidence).
  const modalith::Ray ray{.origin = {4.5, 0.0, 0.0},
                          .direction = modalith::normalized({0.05, 0.0, 1.0})};
  const auto hit = modalith::intersect_surface(ray, asphere);
  // The intersection should succeed — the ray does hit the surface.
  EXPECT_TRUE(hit) << "Grazing-incidence ray on even asphere must converge";
  if (hit) {
    EXPECT_GT(hit.distance, 0.0);
    // Verify the hit point actually lies on the surface.
    const double expected_sag = modalith::surface_sag(
        asphere.profile, hit.point_local.x, hit.point_local.y);
    EXPECT_NEAR(hit.point_local.z, expected_sag, 1.0e-9);
  }
}

TEST(SurfaceIntersection, EvenAsphereHighCurvatureConverges) {
  // Even asphere with high curvature and strong aspheric terms.
  modalith::OpticalSurface asphere;
  asphere.profile.type = modalith::SurfaceType::EvenAsphere;
  asphere.profile.radius = 5.0;
  asphere.profile.conic_constant = -1.5;
  asphere.profile.even_asphere_coefficients = {1.0e-3, 2.0e-5};
  asphere.semi_diameter = 4.0;
  asphere.transform.translation = {0.0, 0.0, 10.0};
  const modalith::Ray ray{.origin = {2.0, 0.0, 0.0}};
  const auto hit = modalith::intersect_surface(ray, asphere);
  EXPECT_TRUE(hit) << "High-curvature even asphere intersection must converge";
  if (hit) {
    const double expected_sag = modalith::surface_sag(
        asphere.profile, hit.point_local.x, hit.point_local.y);
    EXPECT_NEAR(hit.point_local.z, expected_sag, 1.0e-9);
  }
}

TEST(SurfaceIntersection, ApertureBoundaryRayNotRejected) {
  // A ray hitting exactly the semi_diameter boundary should NOT be rejected.
  modalith::OpticalSurface plane;
  plane.transform.translation = {0.0, 0.0, 10.0};
  plane.profile.type = modalith::SurfaceType::Plane;
  plane.semi_diameter = 5.0;
  // Ray arriving exactly at x=5.0 (the semi_diameter edge).
  const modalith::Ray ray{.origin = {5.0, 0.0, 0.0}};
  const auto hit = modalith::intersect_surface(ray, plane);
  EXPECT_TRUE(hit) << "Ray at exact aperture boundary must not be rejected";
  EXPECT_NE(hit.failure, modalith::IntersectionFailure::OutsideAperture);
}

TEST(SurfaceIntersection, ApertureBoundaryMicroOpticsScale) {
  // Micro-optics scale: semi_diameter in µm-equivalent (0.050 mm).
  modalith::OpticalSurface tiny_lens;
  tiny_lens.transform.translation = {0.0, 0.0, 0.1};
  tiny_lens.profile.type = modalith::SurfaceType::Sphere;
  tiny_lens.profile.radius = 0.5;
  tiny_lens.semi_diameter = 0.050;
  // Ray just inside the aperture.
  const modalith::Ray ray{.origin = {0.04999, 0.0, 0.0}};
  const auto hit = modalith::intersect_surface(ray, tiny_lens);
  EXPECT_TRUE(hit) << "Micro-optics ray near aperture boundary must not be falsely rejected";
}
