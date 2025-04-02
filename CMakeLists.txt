cmake_minimum_required(VERSION 3.12)

project(raytracer
	VERSION 1.0
	DESCRIPTION "Just prints Hello world so far"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_executable(raytracer
	src/raytracer.cpp
)

add_executable(colorTest
	test/colors.cpp
)

target_include_directories(raytracer PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_include_directories(colorTest PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_test(NAME colorTest
	COMMAND colorTest
)

