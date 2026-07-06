#include "modalith/analysis/analysis.hpp"
#include "modalith/material/material.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {
modalith::OpticalSurface make_surface(std::string label, double z, double radius,
                                    double semi_diameter,
                                    std::shared_ptr<const modalith::Material> material_after,
                                    bool stop = false) {
  modalith::OpticalSurface surface;
  surface.label = std::move(label);
  surface.transform.translation = {0.0, 0.0, z};
  surface.profile.type = std::isfinite(radius)
      ? modalith::SurfaceType::Sphere : modalith::SurfaceType::Plane;
  surface.profile.radius = radius;
  surface.semi_diameter = semi_diameter;
  surface.material_after = std::move(material_after);
  surface.stop = stop;
  return surface;
}
}  // namespace

int main() {
  modalith::MaterialCatalog catalog;
  modalith::OpticalSystem system;
  system.title = "N-BK7 singlet demonstration";
  system.surfaces = {
    make_surface("Front", 0.0, 50.0, 12.5, catalog.find("N-BK7"), true),
    make_surface("Back", 5.0, -50.0, 12.5, modalith::MaterialCatalog::air()),
    make_surface("Image", 52.0, std::numeric_limits<double>::infinity(), 25.0,
                 modalith::MaterialCatalog::air())
  };

  const modalith::SequentialTracer tracer;
  const modalith::SequentialAnalysis analysis{tracer};
  const auto spot = analysis.spot_diagram(
      system, {}, std::vector<double>{486.1327, 587.5618, 656.2725}, 8);
  const auto fan = analysis.ray_fan(system, {}, 587.5618, 41);

  std::cout << system.title << '\n'
            << "Traced rays: " << spot.samples.size() << '\n'
            << "Vignetted rays: " << spot.vignetted_rays << '\n'
            << std::fixed << std::setprecision(6)
            << "RMS spot radius [mm]: " << spot.rms_radius << '\n'
            << "Geometric spot radius [mm]: " << spot.geometric_radius << '\n'
            << "Ray fan samples: " << fan.samples.size() << '\n';
  return spot.samples.empty() ? 1 : 0;
}
