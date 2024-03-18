cmake_minimum_required(VERSION 3.16)
project(raytracing_hw4)

set(BINARY "${PROJECT_NAME}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -O3")

include_directories(include)
add_executable(${BINARY}
        src/color.cpp
        src/point.cpp
        src/primitives.cpp
        src/quaternion.cpp
        src/scene.cpp
        src/distributions.cpp
        src/main.cpp)

add_subdirectory(glm)
target_link_libraries(${BINARY} glm::glm)