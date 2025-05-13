add_library(pstack_geo INTERFACE)
target_sources(pstack_geo PUBLIC FILE_SET headers TYPE HEADERS FILES
    functions.hpp
    matrix3.hpp
    matrix4.hpp
    point3.hpp
    triangle.hpp
    vector3.hpp
)

set_target_properties(pstack_geo PROPERTIES
    PROJECT_LABEL "geo"
)
target_include_directories(pstack_geo INTERFACE "${PROJECT_SOURCE_DIR}/src")

add_subdirectory(test)
