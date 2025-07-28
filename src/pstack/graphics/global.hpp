#ifndef PSTACK_GRAPHICS_GLOBAL_HPP
#define PSTACK_GRAPHICS_GLOBAL_HPP

#include <expected>
#include <string>

namespace pstack::graphics {

std::expected<void, std::string> initialize();

void viewport(int x, int y, int width, int height);

void clear(float r, float g, float b, float a);

void draw_triangles(int count);

void draw_lines(int count);

} // namespace pstack::graphics

#endif // PSTACK_GRAPHICS_GLOBAL_HPP
