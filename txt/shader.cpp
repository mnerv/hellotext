#include "shader.hpp"
#include <stdexcept>
#include "fmt/format.h"

#include <vector>
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
        auto const err_str = fmt::format("shader link error - ", info_log);
        throw std::runtime_error(err_str);
    }

    glUseProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}
} // namespace txt
