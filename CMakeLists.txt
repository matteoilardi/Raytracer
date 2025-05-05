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

# Set dynamic runtime (/MD or /MDd)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL") # Added to try and cope with Windows build issues

# Set the flag for all targets in the project (ditto)
if (MSVC)
    # MSVC-specific flags for Windows
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MD")
elseif(APPLE)
    # macOS-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif(UNIX)
    # Linux-specific flags (or generic UNIX systems)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")  # or any newer standard if needed
endif()

# Enable CMake's testing framework
enable_testing()

# -------------------------------
# Main executable: raytracer
# -------------------------------

add_executable(raytracer
    src/raytracer.cpp
)

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
