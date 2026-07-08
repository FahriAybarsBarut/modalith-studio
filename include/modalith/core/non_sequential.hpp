#pragma once

#include "modalith/core/geometry.hpp"

#include <cstddef>
#include <memory>
#include <vector>

namespace modalith {

/// Axis-aligned bounding box for spatial acceleration.
struct AABB {
  Vec3 min_point{};
  Vec3 max_point{};

  [[nodiscard]] bool intersects_ray(const Ray& ray) const noexcept;
  [[nodiscard]] bool contains(const Vec3& point) const noexcept;
};

/// Abstract base for any geometry that can participate in
/// non-sequential ray tracing (lenses, mirrors, detectors, baffles, etc.).
class ISceneObject {
 public:
  virtual ~ISceneObject() = default;

  /// Compute the axis-aligned bounding box of this object.
  [[nodiscard]] virtual AABB bounding_box() const = 0;

  /// Intersect a ray with this object. Returns the distance along the ray
  /// to the nearest hit, or std::nullopt if no hit.
  // [[nodiscard]] virtual std::optional<SurfaceIntersection>
  //     intersect(const Ray& ray) const = 0;
};

/// A flat collection of scene objects with BVH-ready layout.
/// This is the entry point for non-sequential tracing.
class Scene {
 public:
  void add_object(std::shared_ptr<ISceneObject> object);
  [[nodiscard]] std::size_t object_count() const noexcept;

  // Future: build_bvh(), trace(ray, max_bounces), etc.

 private:
  std::vector<std::shared_ptr<ISceneObject>> objects_;
};

}  // namespace modalith
