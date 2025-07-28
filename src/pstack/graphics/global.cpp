#include <GL/glew.h> // must be included first
#include "pstack/graphics/global.hpp"

namespace pstack::graphics {

std::expected<void, std::string> initialize() {
    const GLenum err = glewInit();
    if (GLEW_OK != err) {
        return std::unexpected(reinterpret_cast<const char*>(glewGetErrorString(err)));
    }

    glEnable(GL_CULL_FACE);   // Front faces are CCW by default, and back faces are the ones being culled by default
    glEnable(GL_DEPTH_TEST);  // This enables the z buffer, so faces get drawn in order automatically
    // glEnable(GL_LINE_SMOOTH); // This is for anti aliasing of the lines
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return {};
}

void viewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    glViewport(x, y, width, height);
}

void clear(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void draw_triangles(GLsizei count) {
    glDrawArrays(GL_TRIANGLES, 0, count);
}

void draw_lines(GLsizei count) {
    glDrawArrays(GL_LINES, 0, count);
}

} // namespace pstack::graphics
