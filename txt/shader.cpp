#include "shader.hpp"
#include <stdexcept>
#include <vector>

#include "fmt/format.h"
#include "glm/gtc/type_ptr.hpp"

#ifdef __EMSCRIPTEN__
#include "GL/gl.h"
#else
#include "glad/glad.h"
#endif

namespace txt {
auto make_shader(std::string const& vs_src, std::string const& fs_src) -> shader_ref_t {
    return make_ref<shader>(vs_src, fs_src);
}

shader::shader(std::string const& vs_src, std::string const& fs_src) {
    auto vs = compile(GL_VERTEX_SHADER, vs_src.c_str());
    auto fs = compile(GL_FRAGMENT_SHADER, fs_src.c_str());
    m_id = link(vs, fs);
}
shader::~shader() {
    glDeleteProgram(m_id);
}

auto shader::bind() -> void {
    glUseProgram(m_id);
}
auto shader::unbind() -> void {
    glUseProgram(0);
}
auto shader::id() const -> std::uint32_t {
    return m_id;
}


auto shader::upload_num([[maybe_unused]]std::string const& name, [[maybe_unused]]std::uint32_t const& value) -> void {
#ifndef __EMSCRIPTEN__
    glUniform1ui(uniform_location(name), value);
#else
    glUniform1f(uniform_location(name), float(value));
#endif
}
auto shader::upload_num(std::string const& name, std::int32_t const& value) -> void {
    glUniform1i(uniform_location(name), value);
}
auto shader::upload_num(std::string const& name, float const& value) -> void {
    glUniform1f(uniform_location(name), value);
}

auto shader::upload_nums([[maybe_unused]]std::string const& name, [[maybe_unused]]std::int32_t const& count, [[maybe_unused]]std::uint32_t const* values) -> void {
#ifndef __EMSCRIPTEN__
    glUniform1uiv(uniform_location(name), count, values);
#endif
}
auto shader::upload_nums(std::string const& name, std::int32_t const& count, float const* values) -> void {
    glUniform1fv(uniform_location(name), count, values);
}

auto shader::upload_vec2(std::string const& name, glm::vec2 const& value) -> void {
    glUniform2fv(uniform_location(name), 1, glm::value_ptr(value));
}
auto shader::upload_vec3(std::string const& name, glm::vec3 const& value) -> void {
    glUniform3fv(uniform_location(name), 1, glm::value_ptr(value));
}
auto shader::upload_vec4(std::string const& name, glm::vec4 const& value) -> void {
    glUniform4fv(uniform_location(name), 1, glm::value_ptr(value));
}

auto shader::upload_vec2s(std::string const& name, std::int32_t const& count, glm::vec2 const* values) -> void {
    glUniform2fv(uniform_location(name), count, reinterpret_cast<float const*>(values));
}
auto shader::upload_vec3s(std::string const& name, std::int32_t const& count, glm::vec3 const* values) -> void {
    glUniform3fv(uniform_location(name), count, reinterpret_cast<float const*>(values));
}
auto shader::upload_vec4s(std::string const& name, std::int32_t const& count, glm::vec4 const* values) -> void {
    glUniform4fv(uniform_location(name), count, reinterpret_cast<float const*>(values));
}

auto shader::upload_mat2(std::string const& name, glm::mat2 const& value, bool const& transpose) -> void {
    glUniformMatrix2fv(uniform_location(name), 1, (transpose ? GL_TRUE : GL_FALSE), glm::value_ptr(value));
}
auto shader::upload_mat3(std::string const& name, glm::mat3 const& value, bool const& transpose) -> void {
    glUniformMatrix3fv(uniform_location(name), 1, (transpose ? GL_TRUE : GL_FALSE), glm::value_ptr(value));
}
auto shader::upload_mat4(std::string const& name, glm::mat4 const& value, bool const& transpose) -> void {
    glUniformMatrix4fv(uniform_location(name), 1, (transpose ? GL_TRUE : GL_FALSE), glm::value_ptr(value));
}

auto shader::upload_mat2s(std::string const& name, std::int32_t const& count, glm::mat2 const* values, bool const& transpose) -> void {
    glUniformMatrix2fv(uniform_location(name), count, (transpose ? GL_TRUE : GL_FALSE), reinterpret_cast<float const*>(values));
}
auto shader::upload_mat3s(std::string const& name, std::int32_t const& count, glm::mat3 const* values, bool const& transpose) -> void {
    glUniformMatrix3fv(uniform_location(name), count, (transpose ? GL_TRUE : GL_FALSE), reinterpret_cast<float const*>(values));
}
auto shader::upload_mat4s(std::string const& name, std::int32_t const& count, glm::mat4 const* values, bool const& transpose) -> void {
    glUniformMatrix4fv(uniform_location(name), count, (transpose ? GL_TRUE : GL_FALSE), reinterpret_cast<float const*>(values));
}

auto shader::uniform_location(std::string const& name) const -> std::int32_t {
    return glGetUniformLocation(m_id, name.c_str());
}

auto shader::compile(std::uint32_t const& type, char const* source) -> std::uint32_t {
    std::uint32_t program = glCreateShader(type);
    glShaderSource(program, 1, &source, nullptr);
    glCompileShader(program);

    std::int32_t is_success = 0;
    constexpr auto LOG_SIZE = 512;
    static char info_log[LOG_SIZE];
    glGetShaderiv(program, GL_COMPILE_STATUS, &is_success);
    if (!is_success) {
        glGetShaderInfoLog(program, LOG_SIZE, nullptr, info_log);
        auto const err_str = fmt::format("shader compile error [{}] - {}",
            type == GL_VERTEX_SHADER ? "vertex" : "fragment",
            info_log);
        fmt::print("{}\n", err_str);
        throw std::runtime_error(err_str);
    }

    return program;
}

auto shader::link(std::uint32_t const& vs, std::uint32_t const& fs) -> std::uint32_t {
    std::uint32_t program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    std::int32_t is_success = 0;
    constexpr auto LOG_SIZE = 512;
    static char info_log[LOG_SIZE];
    glGetProgramiv(program, GL_LINK_STATUS, &is_success);
    if (!is_success) {
        glGetProgramInfoLog(program, LOG_SIZE, nullptr, info_log);
        auto const err_str = fmt::format("shader link error - {}", info_log);
        throw std::runtime_error(err_str);
    }

    glUseProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}
} // namespace txt
