#ifndef PSTACK_GRAPHICS_SHADER_HPP
#define PSTACK_GRAPHICS_SHADER_HPP

#include <expected>
#include <string>
#include "pstack/geo/matrix4.hpp"
#include "pstack/geo/vector3.hpp"

namespace pstack::graphics {

class shader {
public:
    shader() = default;
    void use_program();

    std::expected<void, std::string> initialize(const char* vertex_source, const char* fragment_source);

    void set_uniform(const char* name, const geo::matrix4<float>& matrix);

private:
    int _program = 0;
};

} // namespace pstack::graphics

#endif // PSTACK_GRAPHICS_SHADER_HPP
