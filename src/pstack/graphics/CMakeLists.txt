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
target_compile_definitions(pstack_graphics PRIVATE
    GLEW_NO_GLU
)

set_target_properties(pstack_graphics PROPERTIES
    PROJECT_LABEL "graphics"
)
target_link_libraries(pstack_graphics
    PUBLIC pstack_geo pstack_util
    PRIVATE glew_s
)
target_include_directories(pstack_graphics PUBLIC "${PROJECT_SOURCE_DIR}/src")
target_include_directories(pstack_graphics PRIVATE "${PROJECT_SOURCE_DIR}/external/glew/include")
