#pragma once

#include "modalith/core/geometry.hpp"
#include "modalith/core/surface.hpp"
#include "modalith/material/material.hpp"
#include <vector>
#include <memory>

namespace modalith::ns {

// Name component for debugging and UI
struct Name {
  std::string value;
};

using Vec3 = modalith::Vec3;
using Transform = modalith::Transform;

// Geometry component (can be a Lens, Mirror, Detector, etc.)
struct OpticalObject {
  SurfaceProfile profile;
  double semi_diameter{};
  std::shared_ptr<const Material> material; // nullptr if reflective or absorbing
  bool is_mirror{false};
  bool is_detector{false};
};

// Bounding box component for BVH
struct AABB {
  Vec3 min_bound;
  Vec3 max_bound;

  bool hit(const Ray& ray, double t_min, double t_max) const;
};

// Detector component
struct DetectorData {
  std::size_t resolution_x{100};
  std::size_t resolution_y{100};
  double physical_width{};
  double physical_height{};
  
  std::vector<double> irradiance_map; // Flattened 2D grid
  
  void init() {
    irradiance_map.assign(resolution_x * resolution_y, 0.0);
  }
};

// Light source component
struct LightSource {
  enum Type {
    Point,
    Directional,
    GaussianBeam
  } type{Point};

  Vec3 direction{0.0, 0.0, 1.0}; // For directional and Gaussian
  double power{1.0};                // Watts
  double wavelength_nm{550.0};
  
  // For Gaussian
  double waist_radius{1.0};
};

} // namespace modalith::ns
