add_executable(pstack_gui WIN32
    app.cpp
    controls.cpp
    list_view.cpp
    main_window.cpp
    parts_list.cpp
    results_list.cpp
    viewport.cpp
)
target_sources(pstack_gui PUBLIC FILE_SET headers TYPE HEADERS FILES
    constants.hpp
    controls.hpp
    list_view.hpp
    main_window.hpp
    parts_list.hpp
    preferences.hpp
    results_list.hpp
    transformation.hpp
    viewport.hpp
)

set_target_properties(pstack_gui PROPERTIES
    PROJECT_LABEL "gui"
)
target_link_libraries(pstack_gui PRIVATE
    wx::net
    wx::core
    wx::base
    wx::gl
    pstack_calc
    pstack_files
    pstack_geo
    pstack_graphics
)
target_include_directories(pstack_gui PRIVATE "${PROJECT_SOURCE_DIR}/src")

set(PSTACK_OUTPUT_TYPE "GUI")
set(PSTACK_OUTPUT_FILE_NAME "PartStackerGUI")
pstack_configure_target_info(pstack_gui)
set_target_properties(pstack_gui PROPERTIES
    OUTPUT_NAME ${PSTACK_OUTPUT_FILE_NAME}
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
)
