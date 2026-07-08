#pragma once

#include "modalith/non_sequential/components.hpp"
#include "modalith/entt/entt.hpp"
#include <vector>

namespace modalith::ns {

class Scene {
public:
  Scene() = default;

  entt::registry& registry() { return registry_; }
  const entt::registry& registry() const { return registry_; }

  entt::entity create_entity(const std::string& name);

  void clear();

  // BVH builder function will be called before tracing
  void build_bvh();

private:
  entt::registry registry_;
};

} // namespace modalith::ns
