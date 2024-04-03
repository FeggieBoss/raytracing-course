cmake_minimum_required(VERSION 3.16)
project(raytracing_hw5)

set(BINARY "${PROJECT_NAME}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -O3")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -O0")

include_directories(include)
add_executable(${BINARY}
        src/color.cpp
        src/point.cpp
        src/primitives.cpp
        src/quaternion.cpp
        src/distributions.cpp
        src/bvh.cpp
        src/scene.cpp
        src/sceneload.cpp
        src/main.cpp)

target_compile_options(${BINARY} PUBLIC -g )

add_subdirectory(glm)
find_package(OpenMP)
target_link_libraries(${BINARY} glm::glm OpenMP::OpenMP_CXX)