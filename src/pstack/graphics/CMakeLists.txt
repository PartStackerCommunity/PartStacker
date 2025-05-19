add_library(pstack_graphics STATIC
    buffer.cpp
    global.cpp
    shader.cpp
)
target_sources(pstack_graphics PUBLIC FILE_SET headers TYPE HEADERS FILES
    buffer.hpp
    global.hpp
    shader.hpp
)

set_target_properties(pstack_graphics PROPERTIES
    PROJECT_LABEL "graphics"
)
target_link_libraries(pstack_graphics
    PUBLIC pstack_geo
    PRIVATE GLEW::GLEW
)
target_include_directories(pstack_graphics PUBLIC "${PROJECT_SOURCE_DIR}/src")
