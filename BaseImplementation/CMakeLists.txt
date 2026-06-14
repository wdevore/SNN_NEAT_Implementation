cmake_minimum_required(VERSION 3.31.0)

project(BaseImplementation VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB DIR_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(BaseImplementation
  ${DIR_SOURCES}
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

target_include_directories(BaseImplementation
    PRIVATE includes
)

target_compile_options(BaseImplementation PRIVATE
    $<$<CONFIG:Debug>:-O0;-g>
)
