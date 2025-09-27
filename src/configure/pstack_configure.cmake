if(DEFINED PSTACK_VERSION_MAJOR OR DEFINED PSTACK_VERSION_MINOR OR DEFINED PSTACK_VERSION_PATCH)
    message(FATAL_ERROR "CMake variables PSTACK_VERSION_MAJOR, PSTACK_VERSION_MINOR, and PSTACK_VERSION_PATCH must not be set by the user.")
endif()

set(PSTACK_VERSION_MAJOR 0)
set(PSTACK_VERSION_MINOR 3)
set(PSTACK_VERSION_PATCH 2)

macro(pstack_configure_file input output)
    configure_file("${PROJECT_SOURCE_DIR}/src/configure/${input}" "${CMAKE_CURRENT_BINARY_DIR}/pstack_generated/${output}")
endmacro()

macro(pstack_configure_target_info target)
    pstack_configure_file("version.hpp.in" "pstack/version.hpp")
    target_include_directories("${target}" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/pstack_generated")
    if(WIN32)
        pstack_configure_file("resource.rc.in" "resource.rc")
        target_sources("${target}" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/pstack_generated/resource.rc")
    elseif(APPLE)
        pstack_configure_file("Info.plist.in"  "Info.plist")
        set_target_properties("${target}" PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/pstack_generated/Info.plist"
        )
    endif()
endmacro()
