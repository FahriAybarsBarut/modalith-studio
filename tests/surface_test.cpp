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
