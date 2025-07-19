cmake_minimum_required(VERSION 3.30)

if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "" FORCE)
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        set(VCPKG_TARGET_TRIPLET "arm64-osx" CACHE STRING "" FORCE)
    else()
        message(FATAL_ERROR "Unsupported platform: ${CMAKE_HOST_SYSTEM_NAME}")
    endif()
endif()

project(PartStacker LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

enable_testing()

find_package(Catch2 CONFIG REQUIRED)
find_package(GLEW REQUIRED)
find_package(mdspan CONFIG REQUIRED)
find_package(wxWidgets CONFIG REQUIRED COMPONENTS core base net gl)

add_subdirectory(src)
