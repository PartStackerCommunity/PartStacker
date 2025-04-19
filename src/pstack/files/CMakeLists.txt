add_library(pstack_files STATIC
    read.cpp
    split.cpp
    stl.cpp
)
target_sources(pstack_files PUBLIC FILE_SET headers TYPE HEADERS FILES
    read.hpp
    split.hpp
    stl.hpp
)

set_target_properties(pstack_files PROPERTIES
    PROJECT_LABEL "files"
)
target_link_libraries(pstack_files
    PUBLIC pstack_geo
)
target_include_directories(pstack_files PUBLIC "${PROJECT_SOURCE_DIR}/src")
