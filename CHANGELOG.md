# Changelog

All notable changes to Modalith Studio are documented here.

## [0.2.1] - 2026-07-07

### Added
- Even Asphere coefficients input column in Lens Data Editor.
- Linear thermo-optic coefficient (dn/dT) for N-BK7, N-F2, and Fused Silica.
- Dynamic reference wavelength bindings in the GUI.

### Fixed
- Ray launch origin coordinate system projection logic on curved surfaces.
- Main optical layout ray drawing starting positions.
- CI configure error by disabling GUI build in GitHub Actions.

## [0.2.0] - 2026-07-06

### Added

- Professional Qt 6/QML optical engineering workspace
- Live Lens Data Editor, optical layout, spot diagram, and ray-fan views
- Versioned `.modalith` project open, save, and save-as workflows
- Legacy `.photon` project import
- Undo/redo and surface insert, delete, and duplicate commands
- Editable wavelength set, temperature, and system title
- CSV export for spot and ray-fan analysis data
- Windows x64 portable and installer release packaging

### Core foundation

- Sequential double-precision ray tracing
- Plane, spherical, conic, and even-asphere surfaces
- Vector Snell refraction, TIR detection, and OPL accumulation
- Sellmeier dispersion with seed glass data
- Polychromatic spot and transverse ray-fan analyses
- Twenty unit, numerical, compatibility, and GUI workflow tests
