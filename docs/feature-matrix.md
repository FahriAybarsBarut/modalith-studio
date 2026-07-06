# Modalith product feature matrix

This matrix is the product backlog and the honest capability boundary for Modalith.
`Implemented` means a user-visible workflow is available and backed by tested
calculation code. `Partial` means a useful subset exists. `Planned` means no
production capability should be implied in the UI yet.

The scope is informed by the public Ansys OpticStudio workflows for
[Sequential Mode](https://optics.ansys.com/hc/en-us/articles/42661713256723),
[Non-Sequential Mode](https://optics.ansys.com/hc/en-us/articles/42661670424851-Exploring-Non-Sequential-Mode-in-OpticStudio),
[MTF analysis](https://optics.ansys.com/hc/en-us/articles/42661772706835-Methods-for-analyzing-MTF-in-OpticStudio),
[polarization](https://optics.ansys.com/hc/en-us/articles/42661755401747),
[tolerancing](https://optics.ansys.com/hc/en-us/articles/43071088477587), and
[automation](https://optics.ansys.com/hc/en-us/articles/42661747915539-ZOS-API-using-Python-NET).

## 1. Project and engineering workflow

| Capability | Status | Delivery target |
|---|---|---|
| Native, versioned project format | Implemented | `.modalith` JSON with atomic save; legacy `.photon` import |
| New, open, save, save-as | Implemented | Desktop file dialogs |
| Dirty-state indicator | Implemented | Window title and save state |
| Undo/redo for prescription edits | Implemented | Snapshot command stack |
| Insert, delete, duplicate surfaces | Implemented | LDE and Edit menu |
| Autosave and crash recovery | Planned | Phase 1 completion |
| Recent files and session restore | Planned | Phase 1 completion |
| Units and locale management | Planned | Phase 1 completion |
| Preferences, keyboard mapping | Planned | Application layer |
| Analysis templates/workspaces | Planned | Application layer |
| PDF/HTML report generation | Planned | Reporting milestone |
| CSV analysis export | Implemented | Spot and ray-fan datasets |
| Plot image/vector export | Planned | Application layer |

## 2. Sequential optical modeling

| Capability | Status | Notes |
|---|---|---|
| Plane, sphere, rotational conic | Implemented | Analytic intersections |
| Even polynomial asphere | Implemented | Newton solve |
| Signed thickness, material, aperture | Implemented | LDE editing |
| Decenter/tilt transforms | Partial | Core transform exists; no coordinate-break editor |
| Coordinate breaks and ordering | Planned | Required for off-axis systems |
| Stop solve and ray aiming | Planned | Required for real entrance pupils |
| Chief, marginal and paraxial rays | Partial | Chief reference only |
| Multiple fields | Planned | Current GUI exposes one field |
| Multiple wavelengths and weights | Partial | Editable wavelength set; equal weighting |
| Multiple configurations/zoom | Planned | MCE-style editor |
| Pickup, solve and variable parameters | Planned | Optimization foundation |
| Odd/extended/Q-type aspheres | Planned | Surface library |
| Zernike/freeform surfaces | Planned | Surface library |
| Toroidal/anamorphic/cylindrical | Planned | Surface library |
| GRIN media | Planned | Material/ray integrator |
| Mirrors, prisms, gratings, DOE | Planned | Sequential modeling |
| Birefringent media | Planned | Polarization milestone |

## 3. Materials, catalogs and coatings

| Capability | Status | Notes |
|---|---|---|
| Sellmeier 3-term dispersion | Implemented | N-BK7, N-F2, fused silica seeds |
| Constant-index material | Implemented | Core API |
| AGF glass catalog import | Planned | No vendor catalog redistribution |
| Schott/Ohara/CDGM/Hoya catalogs | Planned | User-installed catalog support |
| Additional dispersion equations | Planned | Schott, Herzberger, Conrady, Cauchy |
| Temperature-dependent index | Partial | Linear dn/dT hook |
| Bulk transmission and absorption | Planned | Radiometry |
| Thin-film transfer matrices | Planned | Coating milestone |
| Coating catalog/editor/optimizer | Planned | Coating milestone |
| Jones/Mueller polarization | Planned | Polarization milestone |

## 4. Imaging analyses

| Capability | Status | Notes |
|---|---|---|
| 2D optical layout | Implemented | Live YZ section |
| 3D layout and shaded model | Planned | Vulkan/OpenGL view |
| Spot diagram, RMS/GEO radius | Implemented | Polychromatic |
| Tangential/sagittal ray fan | Implemented | Monochromatic reference |
| OPD and wavefront map | Planned | Analysis engine |
| Zernike fit and Seidel breakdown | Planned | Analysis engine |
| FFT/Huygens PSF | Planned | Diffraction milestone |
| Geometric/FFT/Huygens MTF | Planned | Diffraction milestone |
| Through-focus and field MTF | Planned | Diffraction milestone |
| Encircled/ensquared energy | Planned | Analysis engine |
| Distortion/grid distortion | Planned | Analysis engine |
| Field curvature and astigmatism | Planned | Analysis engine |
| Lateral/longitudinal color | Planned | Analysis engine |
| Relative illumination/vignetting | Planned | Analysis engine |
| Geometric image simulation | Planned | Extended-source analysis |
| Interferogram and fringe plots | Planned | Wavefront analysis |

## 5. Optimization and tolerancing

| Capability | Status | Notes |
|---|---|---|
| Merit Function Editor | Planned | Operand framework first |
| Optimization wizard | Planned | Imaging/afocal presets |
| Damped least squares | Planned | Local optimization |
| Orthogonal descent | Planned | Local optimization |
| Global search and Hammer-style refinement | Planned | Global optimization |
| Parameter bounds and variable solves | Planned | Model dependency graph |
| Sensitivity tolerancing | Planned | Manufacturing workflow |
| Monte Carlo tolerancing and yield | Planned | Manufacturing workflow |
| Compensators and inverse tolerancing | Planned | Manufacturing workflow |
| Tolerance wizard and cache | Planned | Manufacturing workflow |
| High-yield/as-built optimization | Planned | Advanced manufacturing |

## 6. Non-sequential, illumination and stray light

| Capability | Status | Notes |
|---|---|---|
| BVH/KD acceleration | Planned | Non-sequential foundation |
| Native 3D solids and Boolean nesting | Planned | Object library |
| CAD mesh import | Planned | STEP/IGES/STL pipeline |
| Point, area, Gaussian, file and IES sources | Planned | Source library |
| Source arrays and measured source models | Planned | Illumination workflow |
| Surface/volume/angle/color detectors | Planned | Detector library |
| Coherent and incoherent accumulation | Planned | Detector engine |
| Ray splitting, diffraction orders | Planned | Path engine |
| Lambertian/Gaussian/ABg/BSDF scatter | Planned | Scatter library |
| Ray database, path filter and flux sorting | Planned | Stray-light workflow |
| Ghost path search | Planned | Stray-light workflow |
| Radiometric and photometric units | Planned | W, lm, cd, lux |
| True-color and spectral detectors | Planned | Color science |
| Mixed sequential/non-sequential mode | Planned | Integration milestone |

## 7. Physical optics and multiphysics

| Capability | Status | Notes |
|---|---|---|
| Fresnel/angular-spectrum propagation | Planned | POP foundation |
| Gaussian beam q-parameter tools | Planned | Laser workflow |
| Coherent propagation through surfaces | Planned | POP workflow |
| Fiber coupling and overlap integrals | Planned | Modalithics workflow |
| Structural/thermal FEA import and fitting | Planned | STOP workflow |
| Rigid-body/higher-order deformation split | Planned | STOP workflow |
| Athermal solve and thermal configurations | Planned | STOP workflow |

## 8. Automation and interoperability

| Capability | Status | Notes |
|---|---|---|
| Headless CLI | Partial | Demonstration system only |
| Python bindings | Planned | pybind11 |
| ZOS-like object model | Planned | `modalith.zos_compat` |
| User operands/extensions | Planned | Stable plugin ABI |
| Batch parameter sweeps | Planned | Headless workflow |
| `.zmx` import/export | Planned | Clean-room format work |
| CODE V `.seq` import | Planned | Optional interoperability |
| STEP/IGES/STL export | Planned | Optomechanical exchange |
| IES/LDT/BSDF/SCA import/export | Planned | Illumination exchange |
| Reproducible reports and audit metadata | Planned | Quality system |

## 9. Acoustic engineering workspace

Acoustics is not an optical surface-type toggle. A credible acoustic workspace
requires its own wave equation, boundary/material models, sources, receivers,
meshing and validation datasets. It remains a separate planned engine:

- geometric room/acoustic ray tracing and image-source methods;
- frequency-dependent absorption, scattering and impedance boundaries;
- directivity balloons, loudspeaker/source data and microphone receivers;
- SPL, impulse response, RT60/EDT/C50/C80/D50/STI analyses;
- diffraction, edge models and hybrid FEM/BEM interfaces;
- auralization and WAV/IR export;
- ISO 3382-style room-acoustic reporting;
- mesh/CAD import and acoustic material catalogs.

The UI may eventually host both engines, but optical results must never be
presented as acoustic calculations.

## Delivery order

1. Finish trustworthy sequential design workflow and golden regressions.
2. Add wavefront/PSF/MTF plus merit-function and DLS foundations.
3. Add tolerancing and manufacturing reports.
4. Build non-sequential illumination/stray-light engine.
5. Add coatings, polarization and physical optics.
6. Add automation/interoperability and GPU acceleration.
7. Start the independently validated acoustic engine.
