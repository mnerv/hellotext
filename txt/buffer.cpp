#include "buffer.hpp"
#include <cassert>

#ifdef __EMSCRIPTEN__
#include "GL/gl.h"
#else
#include "glad/glad.h"
#endif

namespace txt {
buffer::buffer() : m_id(0), m_bytes(0), m_size(0), m_type(0), m_usage(0) {
    glGenBuffers(1, &m_id);
}
buffer::buffer(void const* data, std::size_t bytes, std::size_t size, GLenum type, std::uint32_t usage)
    : m_id(0), m_bytes(bytes), m_size(size), m_type(type), m_usage(usage) {
    glGenBuffers(1, &m_id);
    bind();
    glBufferData(m_type, GLsizeiptr(m_bytes), data, m_usage);
    unbind();
}
buffer::~buffer() {
    glDeleteBuffers(1, &m_id);
}
auto buffer::bind() -> void {
    assert(m_type == 0 && "Unknown buffer type");
    glBindBuffer(m_type, m_id);
}
auto buffer::unbind() -> void {
    assert(m_type == 0 && "Unknown buffer type");
    glBindBuffer(m_type, 0);
}

auto buffer::id() const -> std::uint32_t { return m_id; }
auto buffer::bytes() const -> std::size_t { return m_bytes; }
auto buffer::size() const -> std::size_t { return m_size; }

auto buffer::set(void const* data, std::size_t bytes, std::size_t size, GLenum type, std::uint32_t usage) -> void {
    m_bytes = bytes;
    m_size  = size;
    m_type  = type;
    m_usage = usage;
    bind();
    glBufferData(m_type, GLsizeiptr(m_bytes), data, m_usage);
    unbind();
}
auto buffer::resize(std::size_t bytes) -> void {
    if (bytes == m_bytes) return;
    m_bytes = bytes;
    bind();
    glBufferData(m_type, GLsizeiptr(bytes), nullptr, m_usage);
    unbind();
}
auto buffer::sub(void const* data, std::size_t bytes, std::size_t offset) -> void {
    assert(offset + bytes <= m_bytes);
    bind();
    glBufferSubData(m_type, GLintptr(offset), GLsizeiptr(bytes), data);
    unbind();
}
} // namespace txt
