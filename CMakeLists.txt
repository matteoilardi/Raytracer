# Minimum CMake version
cmake_minimum_required(VERSION 3.12)

# Define the project
project(raytracer
    VERSION 1.0
    DESCRIPTION "Work in progress"
    LANGUAGES CXX
)

# Use C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable CMake's testing framework
enable_testing()

# -------------------------------
# Main executable: raytracer 
# -------------------------------

# Add the main application source
add_executable(raytracer
    src/raytracer.cpp
)


# Include the headers in the "include" directory
target_include_directories(raytracer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# -------------------------------
# Test: colors_test
# -------------------------------

add_executable(colorTest
    test/colors_test.cpp
)

# Make the test target aware of the headers
target_include_directories(colorTest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Register the test so you can run it with `ctest`
add_test(NAME colorTest
    COMMAND colorTest
)

# -------------------------------
# Test: geometry_test
# -------------------------------


# New test for the geometry.hpp file
add_executable(geometryTest
    test/geometry_test.cpp
)

# Make the test target aware of the headers
target_include_directories(geometryTest PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Register this new test too
add_test(NAME geometryTest
    COMMAND geometryTest
)

# -------------------------------
# Test: cameras_test
# -------------------------------


# New test for the geometry.hpp file
add_executable(camerasTest
        test/cameras_test.cpp
)

# Make the test target aware of the headers
target_include_directories(camerasTest PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Register this new test too
add_test(NAME camerasTest
        COMMAND camerasTest
)