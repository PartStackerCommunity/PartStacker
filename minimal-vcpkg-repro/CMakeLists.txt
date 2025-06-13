find_package(wxWidgets CONFIG REQUIRED COMPONENTS gl)
add_executable(minimal-vcpkg-repro repro.cpp)
target_link_libraries(minimal-vcpkg-repro wx::gl)
