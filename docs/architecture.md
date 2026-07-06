# Modalith architecture and delivery map

## Current foundation (0.1.0)

The library boundary is split into geometry/ray, materials, tracing, and analysis.
Public headers contain stable data contracts; numerical implementations stay in
`src`. `Modalith::Core` is intentionally GUI- and GPU-independent, allowing later
Qt, Python, CUDA, and Vulkan layers to consume the same verified CPU reference.

### Surface convention

For a rotational conic with vertex at the local origin,

```text
x² + y² + (1 + k)z² - 2Rz = 0
```

where `R` is signed radius and `k` is the conic constant. Plane, sphere, and
conic intersections are analytic. The quadratic uses Kahan's stable root form
`q = -0.5(b + sign(b)√D)` followed by `q/a` and `c/q`, avoiding catastrophic
cancellation near shallow incidence.

Even aspheres use

```text
z(r) = c r² / (1 + √(1 - (1+k)c²r²)) + A4 r⁴ + A6 r⁶ + ...
```

and solve `ray_z(t) - z(ray_x(t), ray_y(t)) = 0` by Newton iteration with an
analytic radial sag derivative. The iteration is capped, finite-checked, and
uses a double-precision absolute residual threshold.

### Refraction and optical path

Refraction uses vector Snell law with `eta = n_before / n_after`. A negative
transmitted radicand is reported as total internal reflection. Every ray segment
stores both indices, geometric distance, outgoing direction, intercept, and
cumulative optical path length.

Sellmeier dispersion follows the three-term Schott form (TIE-29 convention):

```text
n²(λ) = 1 + Σ Bi λ² / (λ² - Ci), λ in µm
```

### Analysis sampling

Spot diagrams use deterministic concentric rings. Ring `i` contains `6i`
azimuthal points, so a single-wavelength diagram with `R` rings traces
`1 + 3R(R+1)` rays. Centroids use Eigen double-precision vectors. Ray fans trace
both principal pupil axes relative to the chief-ray intercept.

### Complexity

- One sequential ray: `O(S * I)`, with `S` surfaces and `I=1` for analytic
  surfaces or bounded Newton iterations for aspheres.
- Spot diagram: `O(W * R² * S * I)`.
- Ray fan: `O(N * S * I)` with two meridional traces per sample.
- Material lookup: average `O(1)`; Sellmeier evaluation: `O(1)`.

The current tests verify numerical correctness rather than make hardware-specific
throughput claims. Reproducible rays/second benchmarks will enter with the
structure-of-arrays and batch tracer work, before GPU acceleration.

## Validation status

The current GoogleTest suite covers public Schott N-BK7 `n_d`, visible dispersion,
analytic sphere/parabola sag, even-asphere terms, plane/sphere intersection,
aperture rejection, Snell refraction, TIR, parallel-plate invariance,
wavelength-dependent tracing, pupil sampling, spot statistics, and ray fans.

Patent-lens golden data and comparison against independently generated reference
outputs are not present yet. Accordingly, the `<0.1%` cross-tool regression target
is a roadmap requirement, not a current claim.

## Phased roadmap

1. **Phase 1 completion:** ray aiming, explicit coordinate-break ordering,
   odd/freeform/toroidal surfaces, AGF import, golden patent-lens fixtures, batch
   structure-of-arrays tracer, and reproducible performance benchmarks.
2. **Optimization and diffraction:** merit operands, DLS with checked Jacobians,
   pupil OPD, FFT/Huygens PSF, geometric/diffraction MTF, encircled energy.
3. **Non-sequential:** BVH, primitives and tessellated CAD, deterministic RNG,
   Russian roulette, sources, detectors, and scattering/BSDF models.
4. **Physical optics and coatings:** Fresnel/angular-spectrum propagation,
   transfer-matrix coatings, Jones/Mueller polarization.
5. **Tolerance and global search:** sensitivity, Monte Carlo yield, inverse
   tolerancing, Sobol/Hammersley starts, annealing, and genetic search.
6. **Application layer:** Qt6/QML LDE and dockable analyses, 3D renderer,
   versioned native format, legally clean `.zmx` interoperability research,
   pybind11 API, reports, and headless batch workflows.
7. **Acceleration:** profiled CUDA kernels first, Vulkan Compute fallback,
   CPU/GPU cross-validation, deterministic preview/engineering modes.

Each milestone must retain an independently runnable CPU reference path. GPU and
GUI work should not become the authority for numerical truth.
