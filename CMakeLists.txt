cmake_minimum_required(VERSION 3.16)

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()
if ( MSVC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /W4")
endif()

project(PAG LANGUAGES C VERSION "0.0.1")

add_subdirectory(SDL)

add_executable(PAG_bin
    src/main.c
)

target_link_libraries(PAG_bin
    PUBLIC SDL3-shared
)

target_include_directories(PAG_bin
    PUBLIC SDL/include/SDL3
)