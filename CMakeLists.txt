cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

find_package(sword REQUIRED)

project(clibible)

file( GLOB SRCS src/*.cpp)

include_directories(${SWORD_INCLUDE_DIR})

add_executable(clibible ${SRCS})
target_link_libraries(clibible sword)