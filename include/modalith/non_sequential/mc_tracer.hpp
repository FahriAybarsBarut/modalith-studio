#pragma once

#include "modalith/non_sequential/scene.hpp"

namespace modalith::ns {

class MCTracer {
public:
  explicit MCTracer(Scene& scene) : scene_(scene) {}

  /// @brief Traces all rays from light sources in the scene.
  /// @param num_rays The number of rays to shoot per light source.
  /// @param max_bounces The maximum number of intersections per ray.
  void trace_all(std::size_t num_rays = 10000, std::size_t max_bounces = 10);

private:
  Scene& scene_;

  // Returns true if the ray hit something. Updates hit info.
  bool intersect(const Ray& ray, entt::entity& out_entity, double& out_t, Vec3& out_normal) const;
};

} // namespace modalith::ns
