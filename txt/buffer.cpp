#include "buffer.hpp"
#include <cassert>
#include <numeric>
#include <algorithm>

#include "fmt/format.h"

namespace txt {
auto make_vertex_buffer(void const* data, std::size_t const& bytes, txt::type const& type, txt::usage const& usage, attribute_descriptions_t const& layout) -> vertex_buffer_ref_t {
    return make_ref<vertex_buffer>(data, bytes, type, usage, layout);
}
auto make_index_buffer(void const* data, std::size_t const& bytes, std::size_t const& size, txt::type const& type, txt::usage const& usage) -> index_buffer_ref_t {
    return make_ref<index_buffer>(data, bytes, size, type, usage);
}
auto make_attribute_descriptor() -> attribute_descriptor_ref_t {
    return make_ref<attribute_descriptor>();
}

vertex_buffer::vertex_buffer(void const* data, std::size_t const& bytes, txt::type const& type, txt::usage const& usage, attribute_descriptions_t const& layout)
    : m_id(0)
    , m_bytes(bytes)
    , m_type(type)
    , m_usage(usage)
    , m_layout(layout) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(m_bytes), data, gl_usage(m_usage));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
vertex_buffer::~vertex_buffer() {
    glDeleteBuffers(1, &m_id);
}

auto vertex_buffer::bind() const -> void {
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
}
auto vertex_buffer::unbind() const -> void {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

auto vertex_buffer::resize(std::size_t bytes) -> void {
    if (bytes == m_bytes) return;
    m_bytes = bytes;
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(bytes), nullptr, gl_usage(m_usage));
}
auto vertex_buffer::sub(void const* data, std::size_t bytes, std::size_t offset) -> void {
    assert(offset + bytes <= m_bytes);
    glBufferSubData(GL_ARRAY_BUFFER, GLintptr(offset), GLsizeiptr(bytes), data);
}

index_buffer::index_buffer(void const* data, std::size_t const& bytes, std::size_t const& size, txt::type const& type, txt::usage const& usage)
    : m_id(0)
    , m_bytes(bytes)
    , m_size(size)
    , m_type(type)
    , m_usage(usage) {
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(m_bytes), data, gl_usage(m_usage));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
index_buffer::~index_buffer() {
    glDeleteBuffers(1, &m_id);
}

auto index_buffer::bind()   const -> void {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}
auto index_buffer::unbind() const -> void {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

attribute_descriptor::attribute_descriptor() : m_id(0), m_index(0) {
    glGenVertexArrays(1, &m_id);
}
attribute_descriptor::~attribute_descriptor() {
    glDeleteVertexArrays(1, &m_id);
}

auto attribute_descriptor::add(vertex_buffer_ref_t buffer) -> void {
    glBindVertexArray(m_id);
    buffer->bind();

    auto const& layout = buffer->layout();
    auto const stride = compute_stride(layout);
    std::size_t offset = 0;
    std::for_each(std::begin(layout), std::end(layout), [&](attribute_description const& a) {
        auto const index         = GLuint(m_index++);
        auto const size          = gl_component_count(a.format);
        auto const attrib_type   = gl_attribute_type(a.format);
        auto const is_normalised = GLboolean(a.normalized ? GL_TRUE : GL_FALSE);

        glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, size, attrib_type, is_normalised, GLsizei(stride), (void const*)offset);
        glVertexAttribDivisor(index, GLuint(a.divisor));
        offset += gl_type_size(a.format);
    });

    m_buffers.push_back(buffer);
    buffer->unbind();
    glBindVertexArray(0);
}
auto attribute_descriptor::bind()   const -> void {
    for (auto const& buffer : m_buffers) buffer->bind();
    glBindVertexArray(m_id);
}
auto attribute_descriptor::unbind() const -> void {
    for (auto const& buffer : m_buffers) buffer->unbind();
    glBindVertexArray(0);
}

auto attribute_descriptor::compute_stride(attribute_descriptions_t const& layout) -> std::size_t {
   return std::accumulate(std::begin(layout), std::end(layout), std::size_t(0),
    [](auto const& acc, auto const& b) {
        auto const size = gl_type_size(b.format);
        return acc + size;
    });
}

} // namespace txt
