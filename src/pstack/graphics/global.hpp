#ifndef PSTACK_GRAPHICS_GLOBAL_HPP
#define PSTACK_GRAPHICS_GLOBAL_HPP

#include <string>
#include "pstack/util/expected.hpp"

namespace pstack::graphics {

util::expected<void, std::string> initialize();

void viewport(int x, int y, int width, int height);

void clear(float r, float g, float b, float a);

void draw_triangles(int count);

} // namespace pstack::graphics

#endif // PSTACK_GRAPHICS_GLOBAL_HPP
