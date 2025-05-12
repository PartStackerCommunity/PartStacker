add_library(pstack_calc STATIC
    binpack.cpp
    mesh.cpp
    rotations.cpp
    sinterbox.cpp
    stacker.cpp
    voxelize.cpp
)
target_sources(pstack_calc PUBLIC FILE_SET headers TYPE HEADERS FILES
    binpack.hpp
    binpack_types.hpp
    bool.hpp
    mesh.hpp
    part.hpp
    rotations.hpp
    sinterbox.hpp
    stacker_thread.hpp
    stacker.hpp
    voxelize.hpp
)

set_target_properties(pstack_calc PROPERTIES
    PROJECT_LABEL "calc"
)
target_link_libraries(pstack_calc
    PUBLIC pstack_geo pstack_util
)
target_include_directories(pstack_calc PUBLIC "${PROJECT_SOURCE_DIR}/src")
