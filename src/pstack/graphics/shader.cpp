#include <GL/glew.h> // must be included first
#include "pstack/graphics/shader.hpp"

#include <cassert>
#include <string>

namespace pstack::graphics {

namespace {

util::expected<GLuint, std::string> make_shader(const GLchar* const source, const GLenum type) {
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, 1, &source, nullptr);

    GLint result;
    GLchar buffer[1024];

    glCompileShader(handle);
    glGetShaderiv(handle, GL_COMPILE_STATUS, &result);
    if (GL_FALSE == result) {
        glGetShaderInfoLog(handle, std::size(buffer), nullptr, buffer);
        char message[1024];
        std::snprintf(message, std::size(message), "Error compiling shader of type %u: \"%s\"", type, buffer);
        return util::unexpected(message);
    }

    return handle;
}

} // namespace

void shader::use_program() {
    glUseProgram(_program);
}

util::expected<void, std::string> shader::initialize(const char* vertex_source, const char* fragment_source) {
    static_assert(std::same_as<const GLchar*, std::remove_const_t<decltype(vertex_source)>>);
    static_assert(std::same_as<const GLchar*, std::remove_const_t<decltype(fragment_source)>>);

    assert(0 == _program);
    _program = glCreateProgram();

    auto vertex_shader = make_shader(vertex_source, GL_VERTEX_SHADER);
    if (not vertex_shader.has_value()) {
        return util::unexpected(std::move(vertex_shader).error());
    }
    glAttachShader(_program, *vertex_shader);

    auto fragment_shader = make_shader(fragment_source, GL_FRAGMENT_SHADER);
    if (not fragment_shader.has_value()) {
        return util::unexpected(std::move(fragment_shader).error());
    }
    glAttachShader(_program, *fragment_shader);

    GLint result;
    GLchar buffer[1024];

    glLinkProgram(_program);
    glGetProgramiv(_program, GL_LINK_STATUS, &result);
    if (GL_FALSE == result) {
        glGetProgramInfoLog(_program, std::size(buffer), nullptr, buffer);
        char message[1024];
        std::snprintf(message, std::size(message), "Error linking shader program: \"%s\"", buffer);
        return util::unexpected(message);
    }

    // Todo: Move this validation elsewhere.
    // It causes an error on Mac, as it happens too early in the program.
#if 0
    glValidateProgram(_program);
    glGetProgramiv(_program, GL_VALIDATE_STATUS, &result);
    if (GL_FALSE == result) {
        glGetProgramInfoLog(_program, std::size(buffer), nullptr, buffer);
        char message[1024];
        std::snprintf(message, std::size(message), "Invalid shader program: \"%s\"", buffer);
        return util::unexpected(message);
    }
#endif

    glDetachShader(_program, *vertex_shader);
    glDetachShader(_program, *fragment_shader);

    glDeleteShader(*vertex_shader);
    glDeleteShader(*fragment_shader);

    return {};
}

void shader::set_uniform(const char* name, const geo::matrix4<float>& matrix) {
    static_assert(std::same_as<const GLchar*, std::remove_const_t<decltype(name)>>);
    static_assert(sizeof(matrix) == (4 * 4) * sizeof(float));
    GLint location = glGetUniformLocation(_program, name);
    glUniformMatrix4fv(location, 1, GL_TRUE, reinterpret_cast<const GLfloat*>(&matrix));
}

} // namespace pstack::graphics
