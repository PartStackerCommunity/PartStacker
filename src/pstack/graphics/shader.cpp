#include <GL/glew.h> // must be included first
#include "pstack/graphics/shader.hpp"

#include <cassert>
#include <format>

namespace pstack::graphics {

namespace {

std::expected<GLuint, std::string> make_shader(const GLchar* const source, const GLenum type) {
    GLuint handle = glCreateShader(type);
    glShaderSource(handle, 1, &source, nullptr);

    GLint result;
    GLchar buffer[1024];

    glCompileShader(handle);
    glGetShaderiv(handle, GL_COMPILE_STATUS, &result);
    if (GL_FALSE == result) {
        glGetShaderInfoLog(handle, std::size(buffer), nullptr, buffer);
        return std::unexpected(std::format("Error compiling shader of type {}: \"{}\"", type, buffer));
    }

    return handle;
}

} // namespace

void shader::use_program() {
    glUseProgram(_program);
}

std::expected<void, std::string> shader::initialize(const char* vertex_source, const char* fragment_source) {
    static_assert(std::same_as<const GLchar*, std::remove_const_t<decltype(vertex_source)>>);
    static_assert(std::same_as<const GLchar*, std::remove_const_t<decltype(fragment_source)>>);

    assert(0 == _program);
    _program = glCreateProgram();

    auto vertex_shader = make_shader(vertex_source, GL_VERTEX_SHADER);
    if (not vertex_shader.has_value()) {
        return std::unexpected(std::move(vertex_shader).error());
    }
    glAttachShader(_program, *vertex_shader);

    auto fragment_shader = make_shader(fragment_source, GL_FRAGMENT_SHADER);
    if (not fragment_shader.has_value()) {
        return std::unexpected(std::move(fragment_shader).error());
    }
    glAttachShader(_program, *fragment_shader);

    GLint result;
    GLchar buffer[1024];

    glLinkProgram(_program);
    glGetProgramiv(_program, GL_LINK_STATUS, &result);
    if (GL_FALSE == result) {
        glGetProgramInfoLog(_program, std::size(buffer), nullptr, buffer);
        return std::unexpected(std::format("Error linking shader program: \"{}\"", buffer));
    }

	glValidateProgram(_program);
	glGetProgramiv(_program, GL_VALIDATE_STATUS, &result);
	if (GL_FALSE == result) {
        glGetProgramInfoLog(_program, std::size(buffer), nullptr, buffer);
        return std::unexpected(std::format("Invalid shader program: \"{}\"", buffer));
	}

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
