cmake_minimum_required(VERSION 3.31.0)
project(SNN_NEAT_Implemenations)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Prevent Raylib from building examples/games to speed up build
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/BaseImplementation/CMakeLists.txt")
    add_subdirectory(BaseImplementation) 
endif()