cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")

find_path(SWORD_INCLUDE_DIR
          NAMES sword
          PATHS /usr/include
                /usr/include/sword)

project(clibible)

file( GLOB SRCS src/*.cpp)

include_directories(/usr/include /usr/include/sword)

add_executable(clibible ${SRCS})
target_link_libraries(clibible sword)