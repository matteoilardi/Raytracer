# Minimum CMake version
cmake_minimum_required(VERSION 3.14)

# Define the project
project(raytracer
    VERSION 1.0
    DESCRIPTION "Work in progress"
    LANGUAGES CXX
)

option(RAYTRACER_ENABLE_METAL "Enable Metal GPU backend" OFF)

if(RAYTRACER_ENABLE_METAL AND NOT APPLE)
  message(FATAL_ERROR "RAYTRACER_ENABLE_METAL=ON is only supported on Apple platforms")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
  message(STATUS "No build type selected, defaulting to Release")
endif()

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
        src/stb_image_write.cpp
)

if(RAYTRACER_ENABLE_METAL)
    target_compile_definitions(raytracer PRIVATE RAYTRACER_HAS_METAL_BACKEND=1)
else()
    target_compile_definitions(raytracer PRIVATE RAYTRACER_HAS_METAL_BACKEND=0)
endif()

if(RAYTRACER_ENABLE_METAL)
    enable_language(OBJCXX)
    target_sources(raytracer PRIVATE src/metal_backend.mm)
endif()

# Include the headers in the "include" directory
target_include_directories(raytracer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_include_directories(raytracer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
        $<INSTALL_INTERFACE:external>
)

if(RAYTRACER_ENABLE_METAL)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(METAL_FRAMEWORK Metal REQUIRED)

    target_link_libraries(raytracer PRIVATE
            ${FOUNDATION_FRAMEWORK}
            ${METAL_FRAMEWORK}
    )

    target_compile_options(raytracer PRIVATE
            $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>
    )

    set(METAL_SHADER ${CMAKE_CURRENT_SOURCE_DIR}/shaders/raytrace.metal)
    set(METAL_SHADER_HEADERS
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/cameras.metalh
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/geometry.metalh
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/materials.metalh
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/random.metalh
            ${CMAKE_CURRENT_SOURCE_DIR}/shaders/shapes.metalh
    )

    set(METAL_AIR ${CMAKE_CURRENT_BINARY_DIR}/raytrace.air)
    set(METAL_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/raytrace.metallib)

    add_custom_command(
            OUTPUT ${METAL_AIR}
            COMMAND xcrun -sdk macosx metal
                    -I ${CMAKE_CURRENT_SOURCE_DIR}/shaders
                    -c ${METAL_SHADER}
                    -o ${METAL_AIR}
            DEPENDS ${METAL_SHADER} ${METAL_SHADER_HEADERS}
            VERBATIM
    )

    add_custom_command(
            OUTPUT ${METAL_LIBRARY}
            COMMAND xcrun -sdk macosx metallib
                    ${METAL_AIR}
                    -o ${METAL_LIBRARY}
            DEPENDS ${METAL_AIR}
            VERBATIM
    )

    add_custom_target(raytrace_metallib ALL
            DEPENDS ${METAL_LIBRARY}
    )

    add_dependencies(raytracer raytrace_metallib)

    add_custom_command(TARGET raytracer POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${METAL_LIBRARY}
                    $<TARGET_FILE_DIR:raytracer>/raytrace.metallib
    )
endif()

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
