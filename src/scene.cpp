#include "modalith/non_sequential/scene.hpp"

namespace modalith::ns {

entt::entity Scene::create_entity(const std::string& name) {
  auto entity = registry_.create();
  registry_.emplace<Name>(entity, name);
  registry_.emplace<Transform>(entity);
  return entity;
}

void Scene::clear() {
  registry_.clear();
}

void Scene::build_bvh() {
  // A simple placeholder for BVH construction.
  // In a real application, this would gather all AABBs and build a tree.
  
  auto view = registry_.view<OpticalObject, Transform>();
  for (auto entity : view) {
    const auto& obj = view.get<OpticalObject>(entity);
    const auto& tf = view.get<Transform>(entity);
    
    // Estimate AABB
    AABB aabb;
    double r = obj.semi_diameter;
    // VERY rough approximation:
    aabb.min_bound = tf.translation - Vec3{r, r, 0.1};
    aabb.max_bound = tf.translation + Vec3{r, r, 0.1};
    
    registry_.emplace_or_replace<AABB>(entity, aabb);
  }
}

bool AABB::hit(const Ray& ray, double t_min, double t_max) const {
  for (int a = 0; a < 3; ++a) {
    double invD = 1.0;
    double origin = 0.0;
    if (a == 0) { invD = 1.0 / ray.direction.x; origin = ray.origin.x; }
    else if (a == 1) { invD = 1.0 / ray.direction.y; origin = ray.origin.y; }
    else { invD = 1.0 / ray.direction.z; origin = ray.origin.z; }

    double t0 = ((a == 0 ? min_bound.x : (a == 1 ? min_bound.y : min_bound.z)) - origin) * invD;
    double t1 = ((a == 0 ? max_bound.x : (a == 1 ? max_bound.y : max_bound.z)) - origin) * invD;
    if (invD < 0.0) std::swap(t0, t1);
    
    t_min = t0 > t_min ? t0 : t_min;
    t_max = t1 < t_max ? t1 : t_max;
    if (t_max <= t_min) return false;
  }
  return true;
}

} // namespace modalith::ns
