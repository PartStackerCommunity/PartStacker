pstack_add_test_executable(pstack_geo
    functions_ut.cpp
    matrix3_ut.cpp
    point3_ut.cpp
    triangle_ut.cpp
    vector3_ut.cpp
)
target_sources(pstack_geo_test PUBLIC FILE_SET headers TYPE HEADERS FILES
    generate.hpp
)
