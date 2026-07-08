#include "modalith/io/zmx_parser.hpp"
#include "modalith/core/surface.hpp"
#include "modalith/material/material.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

namespace modalith::io {

OpticalSystem parse_zmx(const std::string& filepath, const IGlassCatalogImporter* catalog_importer) {
    OpticalSystem sys;
    sys.title = "Imported from ZMX";

    std::ifstream file(filepath);
    if (!file) {
        throw std::runtime_error("Failed to open ZMX file: " + filepath);
    }

    std::string line;
    OpticalSurface current_surf;
    bool in_surf = false;
    double current_z = 0.0;
    double current_disp = 0.0;

    auto push_surface = [&]() {
        if (in_surf) {
            current_surf.transform.translation.z = current_z;
            sys.surfaces.push_back(current_surf);
            current_z += current_disp;

            // reset for next surface
            current_surf = OpticalSurface();
            current_disp = 0.0;
        }
    };

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "SURF") {
            push_surface();
            in_surf = true;
        } else if (in_surf) {
            if (token == "TYPE") {
                std::string type_str;
                iss >> type_str;
                if (type_str == "STANDARD") {
                    current_surf.profile.type = SurfaceType::Sphere;
                } else if (type_str == "EVENASPH") {
                    current_surf.profile.type = SurfaceType::EvenAsphere;
                } else if (type_str == "COORDBRK") {
                    current_surf.profile.type = SurfaceType::CoordinateBreak;
                }
            } else if (token == "CURV") {
                double curv;
                iss >> curv;
                if (std::abs(curv) > 1e-12) {
                    current_surf.profile.radius = 1.0 / curv;
                } else {
                    current_surf.profile.radius = std::numeric_limits<double>::infinity();
                    if (current_surf.profile.type == SurfaceType::Sphere) {
                        current_surf.profile.type = SurfaceType::Plane;
                    }
                }
            } else if (token == "DISP") {
                iss >> current_disp;
            } else if (token == "DIAM") {
                iss >> current_surf.semi_diameter;
            } else if (token == "CONI") {
                iss >> current_surf.profile.conic_constant;
            } else if (token == "GLAS") {
                std::string glas_name;
                iss >> glas_name;
                if (glas_name == "MIRROR") {
                    current_surf.is_mirror = true;
                    current_surf.material_after = MaterialCatalog::air();
                } else {
                    if (catalog_importer) {
                        auto mat = catalog_importer->find(glas_name);
                        if (mat) {
                            current_surf.material_after = mat;
                        } else {
                            current_surf.material_after = std::make_shared<ConstantIndex>(glas_name, 1.5);
                        }
                    } else {
                        current_surf.material_after = std::make_shared<ConstantIndex>(glas_name, 1.5);
                    }
                }
            } else if (token == "PARM") {
                int index;
                double val;
                iss >> index >> val;
                if (current_surf.profile.type == SurfaceType::EvenAsphere) {
                    if (current_surf.profile.even_asphere_coefficients.size() < static_cast<size_t>(index)) {
                        current_surf.profile.even_asphere_coefficients.resize(index, 0.0);
                    }
                    current_surf.profile.even_asphere_coefficients[index - 1] = val;
                }
            }
        }
    }
    push_surface();

    return sys;
}

} // namespace modalith::io
