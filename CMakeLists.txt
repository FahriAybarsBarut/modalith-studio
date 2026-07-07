cmake_minimum_required(VERSION 3.24)

project(Modalith VERSION 0.2.0 LANGUAGES CXX)

option(MODALITH_BUILD_TESTS "Build Modalith unit and regression tests" ON)
option(MODALITH_BUILD_EXAMPLES "Build Modalith command-line examples" ON)
option(MODALITH_BUILD_GUI "Build the Qt 6 engineering desktop application" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

find_package(Eigen3 3.4 QUIET NO_MODULE)
if(NOT Eigen3_FOUND)
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
)
add_library(Modalith::Core ALIAS modalith_core)

target_include_directories(modalith_core
  PUBLIC
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
  find_package(Qt6 6.5 COMPONENTS Core Gui Qml Quick QuickControls2 QUIET)
  if(NOT Qt6_FOUND)
    message(FATAL_ERROR
      "Qt6 6.5+ not found. Set the QT_DIR environment variable or pass "
      "-DCMAKE_PREFIX_PATH=<qt-install-dir> to CMake. "
      "To build without the GUI, pass -DMODALITH_BUILD_GUI=OFF.")
  endif()
  qt_standard_project_setup(REQUIRES 6.5)
  qt_add_executable(modalith_studio
    apps/gui/main.cpp
    apps/gui/modalith_controller.cpp
    apps/gui/plot_items.cpp
  )
  if(WIN32)
    target_sources(modalith_studio PRIVATE apps/gui/modalith.rc)
  endif()
  qt_add_qml_module(modalith_studio
    URI ModalithApp
    VERSION 1.0
    QML_FILES apps/gui/qml/Main.qml
              apps/gui/qml/ProjectDialogs.qml
  )
  target_include_directories(modalith_studio PRIVATE apps/gui)
  target_link_libraries(modalith_studio PRIVATE
    Modalith::Core
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
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

  add_executable(modalith_tests
    tests/material_test.cpp
    tests/surface_test.cpp
    tests/sequential_tracer_test.cpp
    tests/analysis_test.cpp
  )
  target_link_libraries(modalith_tests PRIVATE Modalith::Core GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(modalith_tests)

  if(TARGET modalith_studio)
    add_executable(modalith_gui_tests
      tests/gui_workflow_test.cpp
      tests/legacy_project_test.cpp
      apps/gui/modalith_controller.cpp
    )
    target_include_directories(modalith_gui_tests PRIVATE apps/gui)
    target_link_libraries(modalith_gui_tests PRIVATE
      Modalith::Core GTest::gtest_main Qt6::Core Qt6::Gui
    )
    if(WIN32)
      find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
      if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET modalith_gui_tests POST_BUILD
          COMMAND ${WINDEPLOYQT_EXECUTABLE}
            --no-compiler-runtime
            --no-translations
            --no-opengl-sw
            $<TARGET_FILE:modalith_gui_tests>
          COMMENT "Running windeployqt for GUI workflow tests"
        )
      else()
        message(WARNING "windeployqt not found; Qt DLLs must be deployed manually for modalith_gui_tests")
      endif()
    else()
      # On non-Windows platforms Qt shared libraries are found via RPATH.
    endif()
    gtest_discover_tests(modalith_gui_tests)
  endif()
endif()
