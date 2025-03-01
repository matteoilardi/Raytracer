cmake_minimum_required(VERSION 3.12)

project(HelloWorld
	VERSION 1.0
	DESCRIPTION "Hello world in C++"
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

add_executable(HelloWorld.exe src/HelloWorld.cpp)
	
