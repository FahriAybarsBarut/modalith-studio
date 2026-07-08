cmake_minimum_required(VERSION 3.24)

project(Modalith VERSION 0.2.1 LANGUAGES CXX)

option(MODALITH_BUILD_TESTS "Build Modalith unit and regression tests" ON)
option(MODALITH_BUILD_EXAMPLES "Build Modalith command-line examples" ON)
option(MODALITH_BUILD_GUI "Build the Qt 6 engineering desktop application" ON)
option(MODALITH_BUILD_PYTHON "Build Python bindings" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)

find_package(Eigen3 3.4 QUIET)
if(NOT TARGET Eigen3::Eigen)
  FetchContent_Declare(
    eigen
    GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
    GIT_TAG 3.4.0
    GIT_SHALLOW TRUE
  )
  set(EIGEN_BUILD_DOC OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(eigen)
endif()

add_library(modalith_core
  src/material.cpp
  src/surface.cpp
  src/sequential_tracer.cpp
  src/analysis.cpp
  src/paraxial.cpp
  src/agf_parser.cpp
  src/physical_optics.cpp
  src/tolerance.cpp
  src/optimization.cpp
  src/math/fft.cpp
  src/scene.cpp
  src/mc_tracer.cpp
  src/zmx_parser.cpp
)
add_library(Modalith::Core ALIAS modalith_core)

target_include_directories(modalith_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(modalith_core PUBLIC Eigen3::Eigen)
target_compile_features(modalith_core PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(modalith_core PRIVATE /W4 /permissive- /EHsc)
else()
  target_compile_options(modalith_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

if(MODALITH_BUILD_EXAMPLES)
  add_executable(modalith_cli apps/modalith_cli.cpp)
  target_link_libraries(modalith_cli PRIVATE Modalith::Core)
endif()

if(MODALITH_BUILD_GUI)
  if(DEFINED ENV{QT_DIR})
    list(PREPEND CMAKE_PREFIX_PATH $ENV{QT_DIR})
  endif()
  find_package(Qt6 6.5 COMPONENTS Core Gui Widgets QUIET)
  if(NOT Qt6_FOUND)
    message(FATAL_ERROR
      "Qt6 6.5+ not found. Set the QT_DIR environment variable or pass "
      "-DCMAKE_PREFIX_PATH=<qt-install-dir> to CMake. "
      "To build without the GUI, pass -DMODALITH_BUILD_GUI=OFF.")
  endif()
  qt_standard_project_setup(REQUIRES 6.5)
  qt_add_executable(modalith_studio
    src/main.cpp
    src/gui/main_window.cpp
    src/gui/lens_data_model.cpp
    src/gui/lens_layout_view.cpp
  )
  if(WIN32)
    target_sources(modalith_studio PRIVATE apps/gui/modalith.rc)
  endif()
  target_include_directories(modalith_studio PRIVATE src)
  target_link_libraries(modalith_studio PRIVATE
    Modalith::Core
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
  )
  set_target_properties(modalith_studio PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

include(CTest)
if(MODALITH_BUILD_TESTS)
  set(BUILD_TESTING ON CACHE BOOL "" FORCE)
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.15.2
      GIT_SHALLOW TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  # -----------------------------------------------------------------------------
  # Tests (GTest)
  # -----------------------------------------------------------------------------
  add_executable(modalith_tests
    tests/material_test.cpp
    tests/material_extensions_test.cpp
    tests/surface_test.cpp
    tests/surface_extensions_test.cpp
    tests/sequential_tracer_test.cpp
    tests/analysis_test.cpp
    tests/paraxial_test.cpp
    tests/physical_optics_test.cpp
    tests/tolerance_test.cpp
    tests/optimization_test.cpp
    tests/ns_test.cpp
    tests/zmx_parser_test.cpp
  )
  target_link_libraries(modalith_tests PRIVATE Modalith::Core GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(modalith_tests)

  if(TARGET modalith_studio)
    # add_executable(modalith_gui_tests
    #   tests/gui_workflow_test.cpp
    #   tests/legacy_project_test.cpp
    #   tests/format_compatibility_test.cpp
    #   apps/gui/modalith_controller.cpp
    # )
  endif()
endif()

if(MODALITH_BUILD_PYTHON)
  find_package(pybind11 CONFIG QUIET)
  if(NOT pybind11_FOUND)
    FetchContent_Declare(
      pybind11
      GIT_REPOSITORY https://github.com/pybind/pybind11.git
      GIT_TAG v2.11.1
      GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(pybind11)
  endif()
  pybind11_add_module(modalith_py src/python/modalith_py.cpp)
  target_link_libraries(modalith_py PRIVATE Modalith::Core)
  set_target_properties(modalith_py PROPERTIES OUTPUT_NAME "modalith")
endif()
