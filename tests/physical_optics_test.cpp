#include <gtest/gtest.h>
#include "modalith/analysis/physical_optics.hpp"
#include "modalith/core/paraxial.hpp"

using namespace modalith;

class PhysicalOpticsTest : public ::testing::Test {
protected:
  OpticalSystem system;

  void SetUp() override {
    system.title = "Simple Focusing System";
    system.wavelengths_nm = { 550.0 };
    system.fields = { {0.0, 0.0} };

    // Entrance pupil / stop
    OpticalSurface stop;
    stop.stop = true;
    stop.semi_diameter = 5.0; // 10mm pupil
    stop.transform.translation.z = 0.0;
    system.surfaces.push_back(stop);

    // Ideal lens (paraxial) using thin lens approximation or a single refractive surface
    // To make it simple, we use a single Plano-Convex lens
    OpticalSurface s1;
    s1.profile.radius = 50.0; // R = 50mm
    s1.semi_diameter = 6.0;
    MaterialCatalog catalog;
    s1.material_after = catalog.find("N-BK7"); // n ~ 1.5168 at 550nm
    s1.transform.translation.z = 10.0;
    system.surfaces.push_back(s1);

    OpticalSurface s2;
    s2.profile.radius = std::numeric_limits<double>::infinity(); // Plano
    s2.semi_diameter = 6.0;
    s2.transform.translation.z = 15.0;
    system.surfaces.push_back(s2);

    // Image plane
    ParaxialCalculator paraxial(system, 550.0);
    ParaxialProperties props = paraxial.compute();
    
    OpticalSurface img;
    img.transform.translation.z = 15.0 + props.bfl;
    system.surfaces.push_back(img);
  }
};

TEST_F(PhysicalOpticsTest, ComputePupilFunction) {
  SequentialTracer tracer;
  PhysicalOptics po(tracer);

  ComplexField pupil = po.compute_pupil_function(system, system.fields[0], 550.0, 64);
  EXPECT_EQ(pupil.width, 64);
  EXPECT_EQ(pupil.height, 64);
  EXPECT_GT(pupil.dx, 0.0);
  EXPECT_GT(pupil.dy, 0.0);
  
  // At least some points should have non-zero amplitude
  bool has_light = false;
  for (const auto& val : pupil.data) {
    if (std::abs(val) > 0.0) {
      has_light = true;
      break;
    }
  }
  EXPECT_TRUE(has_light);
}

TEST_F(PhysicalOpticsTest, ComputePSF) {
  SequentialTracer tracer;
  PhysicalOptics po(tracer);

  ParaxialCalculator paraxial(system, 550.0);
  ParaxialProperties props = paraxial.compute();

  ComplexField pupil = po.compute_pupil_function(system, system.fields[0], 550.0, 64);
  ComplexField psf = po.compute_psf(pupil, 550.0, props.efl);

  EXPECT_EQ(psf.width, 64);
  EXPECT_EQ(psf.height, 64);
  EXPECT_GT(psf.dx, 0.0);

  // Center of PSF should have high intensity
  std::size_t center_idx = (64 / 2) * 64 + (64 / 2);
  EXPECT_NEAR(std::abs(psf.data[center_idx]), 1.0, 0.01); // Peak is normalized to 1.0
}

TEST_F(PhysicalOpticsTest, ComputeMTF) {
  SequentialTracer tracer;
  PhysicalOptics po(tracer);

  ParaxialCalculator paraxial(system, 550.0);
  ParaxialProperties props = paraxial.compute();

  ComplexField pupil = po.compute_pupil_function(system, system.fields[0], 550.0, 64);
  ComplexField psf = po.compute_psf(pupil, 550.0, props.efl);
  MTFData mtf = po.compute_mtf(psf, 550.0, props.efl);

  EXPECT_GT(mtf.cutoff_frequency, 0.0);
  EXPECT_GT(mtf.frequencies_cycles_per_mm.size(), 1);
  EXPECT_EQ(mtf.frequencies_cycles_per_mm.size(), mtf.tangential_mtf.size());
  EXPECT_EQ(mtf.frequencies_cycles_per_mm.size(), mtf.sagittal_mtf.size());

  // MTF at DC (freq=0) should be 1.0
  EXPECT_NEAR(mtf.tangential_mtf[0], 1.0, 0.01);
  EXPECT_NEAR(mtf.sagittal_mtf[0], 1.0, 0.01);

  // MTF should decrease with frequency for an aberrated system
  EXPECT_LT(mtf.tangential_mtf.back(), 1.0);
}

TEST_F(PhysicalOpticsTest, TraceGaussianBeam) {
  SequentialTracer tracer;
  PhysicalOptics po(tracer);

  GaussianBeam input_beam;
  input_beam.wavelength_nm = 550.0;
  input_beam.waist_radius = 1.0; // 1 mm waist radius
  input_beam.waist_z = 0.0; // waist exactly at the first surface

  std::vector<GaussianBeam> beams = po.trace_gaussian_beam(system, input_beam);

  // We have 3 surfaces (stop, s1, s2, img), so trace returns size == 4?
  // Actually system.surfaces has 4 surfaces (stop, s1, s2, img).
  // The first element is input_beam. Then for each surface it adds one. So 1 + 4 = 5 elements.
  EXPECT_EQ(beams.size(), 5);

  // At the image plane (the last surface), the beam should be somewhat focused since it passed through a positive lens
  GaussianBeam final_beam = beams.back();
  EXPECT_GT(final_beam.waist_radius, 0.0);
  EXPECT_LT(final_beam.waist_radius, 10.0);
}
