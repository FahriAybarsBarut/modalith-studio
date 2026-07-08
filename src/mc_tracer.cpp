#include "modalith/non_sequential/mc_tracer.hpp"
#include <random>
#include <iostream>
#include <numbers>

namespace modalith::ns {

bool MCTracer::intersect(const Ray& ray, entt::entity& out_entity, double& out_t, Vec3& out_normal) const {
  const auto& reg = scene_.registry();
  auto view = reg.view<OpticalObject, Transform, AABB>();

  double closest_t = std::numeric_limits<double>::infinity();
  entt::entity closest_entity = entt::null;
  Vec3 closest_normal;

  for (auto entity : view) {
    const auto& aabb = view.get<AABB>(entity);
    if (!aabb.hit(ray, 0.001, closest_t)) {
      continue;
    }

    const auto& obj = view.get<OpticalObject>(entity);
    const auto& tf = view.get<Transform>(entity);

    OpticalSurface temp_surf;
    temp_surf.profile = obj.profile;
    temp_surf.transform = tf;
    temp_surf.semi_diameter = obj.semi_diameter;
    temp_surf.is_mirror = obj.is_mirror;

    auto intersection = intersect_surface(ray, temp_surf);
    
    if (intersection && intersection.distance > 1e-9 && intersection.distance < closest_t) {
      closest_t = intersection.distance;
      closest_entity = entity;
      closest_normal = intersection.normal_global;
    }
  }

  if (closest_entity != entt::null) {
    out_entity = closest_entity;
    out_t = closest_t;
    // Transform normal to world space (assuming no rotation)
    out_normal = closest_normal; 
    return true;
  }
  
  return false;
}

void MCTracer::trace_all(std::size_t num_rays, std::size_t max_bounces) {
  scene_.build_bvh(); // Ensure AABBs are ready
  
  auto& reg = scene_.registry();
  auto light_view = reg.view<LightSource, Transform>();
  
  std::mt19937 rng(1337);
  std::uniform_real_distribution<double> dist_u(0.0, 1.0);
  std::uniform_real_distribution<double> dist_v(0.0, 1.0);

  for (auto entity_l : light_view) {
    const auto& light = light_view.get<LightSource>(entity_l);
    const auto& tf_l = light_view.get<Transform>(entity_l);

    double power_per_ray = light.power / num_rays;

    for (std::size_t r = 0; r < num_rays; ++r) {
      Ray ray;
      ray.origin = tf_l.translation;
      
      // Simple cosine-weighted hemisphere sampling for Point light
      if (light.type == LightSource::Point) {
        double theta = std::acos(std::sqrt(dist_u(rng)));
        double phi = 2.0 * std::numbers::pi * dist_v(rng);
        ray.direction = Vec3{
          std::sin(theta) * std::cos(phi),
          std::sin(theta) * std::sin(phi),
          std::cos(theta)
        };
      } else {
        ray.direction = light.direction;
      }
      
      ray.direction = normalized(ray.direction);

      double current_n = MaterialCatalog::air()->refractive_index(light.wavelength_nm, 20.0);
      double ray_power = power_per_ray;

      for (std::size_t b = 0; b < max_bounces; ++b) {
        entt::entity hit_entity;
        double t_hit;
        Vec3 normal;

        if (!intersect(ray, hit_entity, t_hit, normal)) {
          break; // Ray escaped
        }

        const auto& obj = reg.get<OpticalObject>(hit_entity);
        const auto& tf_obj = reg.get<Transform>(hit_entity);
        
        Vec3 hit_point = ray.origin + ray.direction * t_hit;

        if (obj.is_detector && reg.all_of<DetectorData>(hit_entity)) {
          auto& det = reg.get<DetectorData>(hit_entity);
          // Map local coordinates to pixel
          Vec3 local_hit = hit_point - tf_obj.translation;
          double u = (local_hit.x + det.physical_width * 0.5) / det.physical_width;
          double v = (local_hit.y + det.physical_height * 0.5) / det.physical_height;
          
          if (u >= 0.0 && u < 1.0 && v >= 0.0 && v < 1.0) {
            std::size_t px = static_cast<std::size_t>(u * det.resolution_x);
            std::size_t py = static_cast<std::size_t>(v * det.resolution_y);
            // Add power to pixel
            det.irradiance_map[py * det.resolution_x + px] += ray_power;
          }
          break; // Stop at detector
        }

        if (obj.is_mirror) {
          // Reflection
          ray.origin = hit_point + normal * 1e-4; // offset
          ray.direction = ray.direction - normal * (2.0 * dot(ray.direction, normal));
          ray.direction = normalized(ray.direction);
        } else {
          // Refraction
          double n_next = MaterialCatalog::air()->refractive_index(light.wavelength_nm, 20.0);
          if (obj.material) {
             n_next = obj.material->refractive_index(light.wavelength_nm, 20.0);
          }
          
          // Using Snell's law vector form
          // Note: we need to handle inside/outside surface normals correctly
          Vec3 n_unit = normalized(normal);
          double cosi = std::clamp(dot(ray.direction, n_unit), -1.0, 1.0);
          double eta = current_n / n_next;
          
          if (cosi > 0.0) {
            // Ray is hitting the surface from the inside
            n_unit = n_unit * -1.0;
            eta = current_n / MaterialCatalog::air()->refractive_index(light.wavelength_nm, 20.0); // Simplified back to air
          } else {
            cosi = -cosi;
          }

          double k = 1.0 - eta * eta * (1.0 - cosi * cosi);
          
          if (k < 0.0) {
            // Total Internal Reflection
            ray.origin = hit_point + n_unit * 1e-4;
            ray.direction = ray.direction - n_unit * (-2.0 * dot(ray.direction, n_unit));
          } else {
            // Refraction
            ray.origin = hit_point - n_unit * 1e-4;
            ray.direction = ray.direction * eta + n_unit * (eta * cosi - std::sqrt(k));
            current_n = n_next;
          }
          ray.direction = normalized(ray.direction);
        }
      }
    }
  }
}

} // namespace modalith::ns
