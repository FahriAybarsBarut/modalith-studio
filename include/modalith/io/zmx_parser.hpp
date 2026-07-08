#pragma once

#include "modalith/core/sequential_tracer.hpp"
#include "modalith/material/material.hpp"
#include "modalith/core/glass_catalog.hpp"

#include <string>

namespace modalith::io {

/// Parse a Zemax (.zmx) file and return an OpticalSystem.
/// Supported commands: SURF, TYPE, CURV, DISP, GLAS, CONI, PARM, DIAM.
/// If `catalog_importer` is provided, it will be used to resolve glass names.
OpticalSystem parse_zmx(const std::string& filepath, const IGlassCatalogImporter* catalog_importer = nullptr);

} // namespace modalith::io
