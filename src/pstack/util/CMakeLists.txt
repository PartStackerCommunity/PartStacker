add_library(pstack_util INTERFACE)
target_sources(pstack_util PUBLIC FILE_SET headers TYPE HEADERS FILES
    mdarray.hpp
)

set_target_properties(pstack_util PROPERTIES
    PROJECT_LABEL "util"
)
target_link_libraries(pstack_util INTERFACE std::mdspan)
