cmake_minimum_required(VERSION 3.16)
project(raytracing_hw1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -O2 -fopenmp")

include_directories(include)
add_executable(raytracing_hw1
        src/color.cpp
        src/point.cpp
        src/primitives.cpp
        src/quaternion.cpp
        src/scene.cpp
        src/main.cpp)