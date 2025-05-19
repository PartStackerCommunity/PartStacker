macro(pstack_add_test_executable target)
    add_executable("${target}_test"
        ${ARGN}
    )
    target_link_libraries("${target}_test" PRIVATE
        "${target}"
        Catch2::Catch2WithMain
    )
    set_target_properties("${target}_test" PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/test"
    )
    include("${Catch2_DIR}/Catch.cmake")
    catch_discover_tests("${target}_test")
endmacro()

add_subdirectory(calc)
add_subdirectory(files)
add_subdirectory(geo)
add_subdirectory(graphics)
add_subdirectory(gui)
add_subdirectory(util)
