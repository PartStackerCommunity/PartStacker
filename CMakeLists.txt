cmake_minimum_required(VERSION 3.23)

project(PartStacker LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

enable_testing()

add_subdirectory(external)
add_subdirectory(src)
