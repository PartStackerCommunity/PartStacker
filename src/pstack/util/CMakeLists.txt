add_library(pstack_util INTERFACE)
target_sources(pstack_util PUBLIC FILE_SET headers TYPE HEADERS FILES
    expected.hpp
    mdarray.hpp
)

set_target_properties(pstack_util PROPERTIES
    PROJECT_LABEL "util"
)
target_include_directories(pstack_util INTERFACE
    "${PROJECT_SOURCE_DIR}/src"
    "${PROJECT_SOURCE_DIR}/external/mdspan/include"
)
