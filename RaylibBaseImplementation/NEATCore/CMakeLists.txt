cmake_minimum_required(VERSION 3.31.0)

file(GLOB DIR_SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(NEATCoreLib STATIC
    ${DIR_SOURCES}
)

target_include_directories(NEATCoreLib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

# target_link_libraries(NEATCoreLib
#   PUBLIC raylib
# )