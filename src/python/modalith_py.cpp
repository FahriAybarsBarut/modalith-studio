#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "modalith/core/geometry.hpp"
#include "modalith/core/surface.hpp"
#include "modalith/core/sequential_tracer.hpp"
#include "modalith/non_sequential/scene.hpp"
#include "modalith/non_sequential/mc_tracer.hpp"

namespace py = pybind11;
using namespace modalith;

PYBIND11_MODULE(modalith, m) {
    m.doc() = "Modalith Python API";

    py::class_<Vec3>(m, "Vec3")
        .def(py::init<>())
        .def(py::init<double, double, double>())
        .def_readwrite("x", &Vec3::x)
        .def_readwrite("y", &Vec3::y)
        .def_readwrite("z", &Vec3::z);

    py::class_<Ray>(m, "Ray")
        .def(py::init<>())
        .def_readwrite("origin", &Ray::origin)
        .def_readwrite("direction", &Ray::direction)
        .def_readwrite("wavelength_nm", &Ray::wavelength_nm)
        .def_readwrite("intensity", &Ray::intensity)
        .def_readwrite("optical_path_length", &Ray::optical_path_length);

    py::enum_<SurfaceType>(m, "SurfaceType")
        .value("Plane", SurfaceType::Plane)
        .value("Sphere", SurfaceType::Sphere)
        .value("Conic", SurfaceType::Conic)
        .value("EvenAsphere", SurfaceType::EvenAsphere)
        .value("OddAsphere", SurfaceType::OddAsphere)
        .value("ZernikeSag", SurfaceType::ZernikeSag)
        .value("CoordinateBreak", SurfaceType::CoordinateBreak)
        .value("Mirror", SurfaceType::Mirror)
        .value("Biconic", SurfaceType::Biconic)
        .export_values();

    py::class_<SurfaceProfile>(m, "SurfaceProfile")
        .def(py::init<>())
        .def_readwrite("type", &SurfaceProfile::type)
        .def_readwrite("radius", &SurfaceProfile::radius)
        .def_readwrite("conic_constant", &SurfaceProfile::conic_constant)
        .def_readwrite("radius_x", &SurfaceProfile::radius_x)
        .def_readwrite("conic_x", &SurfaceProfile::conic_x)
        .def_readwrite("even_asphere_coefficients", &SurfaceProfile::even_asphere_coefficients)
        .def_readwrite("odd_asphere_coefficients", &SurfaceProfile::odd_asphere_coefficients)
        .def_readwrite("zernike_coefficients", &SurfaceProfile::zernike_coefficients)
        .def_readwrite("zernike_norm_radius", &SurfaceProfile::zernike_norm_radius);

    py::class_<Mat3>(m, "Mat3")
        .def(py::init<>())
        .def_static("from_euler_xyz", &Mat3::from_euler_xyz);

    py::class_<Transform>(m, "Transform")
        .def(py::init<>())
        .def_readwrite("rotation", &Transform::rotation)
        .def_readwrite("translation", &Transform::translation);

    py::class_<OpticalSurface>(m, "OpticalSurface")
        .def(py::init<>())
        .def_readwrite("label", &OpticalSurface::label)
        .def_readwrite("transform", &OpticalSurface::transform)
        .def_readwrite("profile", &OpticalSurface::profile)
        .def_readwrite("semi_diameter", &OpticalSurface::semi_diameter)
        .def_readwrite("stop", &OpticalSurface::stop)
        .def_readwrite("is_mirror", &OpticalSurface::is_mirror);

    py::class_<FieldAngle>(m, "FieldAngle")
        .def(py::init<>())
        .def_readwrite("x_degrees", &FieldAngle::x_degrees)
        .def_readwrite("y_degrees", &FieldAngle::y_degrees);

    py::class_<OpticalSystem>(m, "OpticalSystem")
        .def(py::init<>())
        .def_readwrite("title", &OpticalSystem::title)
        .def_readwrite("surfaces", &OpticalSystem::surfaces)
        .def_readwrite("fields", &OpticalSystem::fields)
        .def_readwrite("wavelengths_nm", &OpticalSystem::wavelengths_nm)
        .def_readwrite("primary_wavelength_index", &OpticalSystem::primary_wavelength_index)
        .def_readwrite("temperature_c", &OpticalSystem::temperature_c);

    py::enum_<TraceFailure>(m, "TraceFailure")
        .value("None", TraceFailure::None)
        .value("MissedSurface", TraceFailure::MissedSurface)
        .value("Vignetted", TraceFailure::Vignetted)
        .value("TotalInternalReflection", TraceFailure::TotalInternalReflection)
        .value("InvalidIndex", TraceFailure::InvalidIndex)
        .export_values();

    py::class_<RaySegment>(m, "RaySegment")
        .def_readwrite("surface_index", &RaySegment::surface_index)
        .def_readwrite("intercept", &RaySegment::intercept)
        .def_readwrite("direction_after", &RaySegment::direction_after)
        .def_readwrite("refractive_index_before", &RaySegment::refractive_index_before)
        .def_readwrite("refractive_index_after", &RaySegment::refractive_index_after)
        .def_readwrite("geometric_length", &RaySegment::geometric_length)
        .def_readwrite("optical_path_length", &RaySegment::optical_path_length);

    py::class_<TraceResult>(m, "TraceResult")
        .def_readwrite("ray", &TraceResult::ray)
        .def_readwrite("segments", &TraceResult::segments)
        .def_readwrite("failure", &TraceResult::failure)
        .def_readwrite("failed_surface", &TraceResult::failed_surface)
        .def("__bool__", [](const TraceResult& tr) { return static_cast<bool>(tr); });

    py::class_<SequentialTracer>(m, "SequentialTracer")
        .def(py::init<>())
        .def("trace", &SequentialTracer::trace);

    py::module ns = m.def_submodule("ns", "Non-sequential module");

    py::class_<ns::Scene>(ns, "Scene")
        .def(py::init<>())
        .def("create_entity", [](ns::Scene& s, const std::string& name) {
            return static_cast<uint32_t>(s.create_entity(name));
        })
        .def("clear", &ns::Scene::clear)
        .def("build_bvh", &ns::Scene::build_bvh);

    py::class_<ns::MCTracer>(ns, "MCTracer")
        .def(py::init<ns::Scene&>())
        .def("trace_all", &ns::MCTracer::trace_all, py::arg("num_rays") = 10000, py::arg("max_bounces") = 10);
}
