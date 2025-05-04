# Minimum CMake version
cmake_minimum_required(VERSION 3.14)

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

add_executable(raytracer
        src/raytracer.cpp
)

# Add flags to avoid deprecated warnings
set_source_files_properties(
        src/raytracer.cpp
        PROPERTIES
        COMPILE_FLAGS "-Wno-deprecated-declarations"
)

# Include the headers in the "include" directory
target_include_directories(raytracer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# -------------------------------
# Add GoogleTest from external/
# -------------------------------

add_subdirectory(external/googletest)
include(GoogleTest)

# -------------------------------
# Helper function to add gtest-based tests
# -------------------------------

function(add_gtest TEST_NAME TEST_FILE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_include_directories(${TEST_NAME} PUBLIC include)
    target_link_libraries(${TEST_NAME} gtest_main)
    gtest_discover_tests(${TEST_NAME})
endfunction()

# -------------------------------
# Register your test files here
# -------------------------------

add_gtest(colorTest test/colors_Gtest.cpp)
add_gtest(geometryTest test/geometry_Gtest.cpp)
add_gtest(camerasTest test/cameras_Gtest.cpp)
add_gtest(shapesTest test/shapes_Gtest.cpp)
