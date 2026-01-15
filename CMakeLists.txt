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

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)

# Ensure GTest uses shared C runtime (/MD on windows) to avoid linking errors
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Enable CMake's testing framework
enable_testing()

# -------------------------------
# Main executable: raytracer
# -------------------------------

add_executable(raytracer
        src/raytracer.cpp
)

# Include the headers in the "include" directory
target_include_directories(raytracer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_include_directories(raytracer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
        $<INSTALL_INTERFACE:external>
)

# -------------------------------
# Add GoogleTest
# -------------------------------

include(GoogleTest)

# -------------------------------
# Helper function to add gtest-based tests
# -------------------------------

function(add_gtest TEST_NAME TEST_FILE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_include_directories(${TEST_NAME} PUBLIC include)
    target_include_directories(${TEST_NAME} PUBLIC external)
    target_link_libraries(${TEST_NAME} gtest_main)
    gtest_discover_tests(${TEST_NAME})
endfunction()

# -------------------------------
# Register test files
# -------------------------------

add_gtest(colorTest test/colors_test.cpp)
add_gtest(geometryTest test/geometry_test.cpp)
add_gtest(camerasTest test/cameras_test.cpp)
add_gtest(shapesTest test/shapes_test.cpp)
add_gtest(randomTest test/random_test.cpp)
add_gtest(materialsTest test/materials_test.cpp)
add_gtest(renderersTest test/renderers_test.cpp)
add_gtest(scenefilesTest test/scenefiles_test.cpp)

# -------------------------------
# Disable warnings due to deprecated functions in library include/stb_image_write.h
# -------------------------------


foreach(tgt raytracer colorTest geometryTest camerasTest shapesTest randomTest materialsTest renderersTest scenefilesTest)
    if(MSVC)
        target_compile_options(${tgt} PRIVATE /wd4996)
    else()
        target_compile_options(${tgt} PRIVATE -Wno-deprecated-declarations)
    endif()
endforeach()
