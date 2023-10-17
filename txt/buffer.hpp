#ifndef TXT_BUFFER_HPP
#define TXT_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include "utility.hpp"

#ifdef __EMSCRIPTEN__
#include "GL/gl.h"
#else
#include "glad/glad.h"
#endif

namespace txt {
// OpenGL Type: https://www.khronos.org/opengl/wiki/OpenGL_Type
enum class type : std::uint32_t {
    unknown = 0,
    boolean,
    i8,    u8,
    i16,   u16,
    i32,   u32,   p32,
    i64,   u64,   p64,
    f16,   f32,   f64,
    vec2,  vec3,  vec4,
    ivec2, ivec3, ivec4,
    dvec2, dvec3, dvec4,
    mat2,  mat3,  mat4,
};

enum class usage : std::uint32_t {
    stream_draw,
    stream_read,
    stream_copy,
    static_draw,
    static_read,
    static_copy,
    dynamic_draw,
    dynamic_read,
    dynamic_copy
};

inline constexpr auto gl_usage(txt::usage const& usage) -> GLenum {
    switch (usage) {
        case txt::usage::stream_draw:  return GL_STREAM_DRAW;
        case txt::usage::stream_read:  return GL_STREAM_READ;
        case txt::usage::stream_copy:  return GL_STREAM_COPY;
        case txt::usage::static_draw:  return GL_STATIC_DRAW;
        case txt::usage::static_read:  return GL_STATIC_READ;
        case txt::usage::static_copy:  return GL_STATIC_COPY;
        case txt::usage::dynamic_draw: return GL_DYNAMIC_DRAW;
        case txt::usage::dynamic_read: return GL_DYNAMIC_READ;
        case txt::usage::dynamic_copy: return GL_DYNAMIC_COPY;
        default: return GL_STATIC_DRAW;
    }
}

inline constexpr auto gl_attribute_type(txt::type const& type) -> GLenum {
    switch (type) {
        case txt::type::i8:    return GL_BYTE;
        case txt::type::i16:   return GL_SHORT;
        case txt::type::i32:
        case txt::type::ivec2:
        case txt::type::ivec3:
        case txt::type::ivec4: return GL_INT;
        case txt::type::u8:    return GL_UNSIGNED_BYTE;
        case txt::type::u16:   return GL_UNSIGNED_SHORT;
        case txt::type::u32:   return GL_UNSIGNED_INT;
        case txt::type::f64:   return GL_DOUBLE;
        default:               return GL_FLOAT;
    }
}

inline constexpr auto gl_component_count(txt::type const& type) -> GLint {
    switch(type) {
        case txt::type::vec2:
        case txt::type::ivec2:
        case txt::type::dvec2: return 2;

        case txt::type::vec3:
        case txt::type::ivec3:
        case txt::type::dvec3: return 3;

        case txt::type::vec4:
        case txt::type::ivec4:
        case txt::type::dvec4: return 4;

        case txt::type::mat2:  return 2 * 2;
        case txt::type::mat3:  return 3 * 3;
        case txt::type::mat4:  return 4 * 4;

        default: return 1;
    }
}

inline constexpr auto gl_type(txt::type const& type) -> GLenum {
    switch (type) {
        case txt::type::boolean: return GL_BOOL;
        case txt::type::u8:      return GL_UNSIGNED_BYTE;
        case txt::type::u16:     return GL_UNSIGNED_SHORT;
        case txt::type::u32:     return GL_UNSIGNED_INT;
        case txt::type::i8:      return GL_BYTE;
        case txt::type::i16:     return GL_SHORT;
        case txt::type::i32:     return GL_INT;
        default: return GL_UNSIGNED_INT;
    }
}

// type size in bytes
inline constexpr auto gl_type_size(txt::type const& type) -> std::size_t {
    switch(type) {
        case txt::type::boolean:
        case txt::type::i8:
        case txt::type::u8:    return 1;

        case txt::type::i16:
        case txt::type::u16:
        case txt::type::f16:   return 2;

        case txt::type::i32:
        case txt::type::u32:
        case txt::type::p32:
        case txt::type::f32:   return 4;

        case txt::type::i64:
        case txt::type::u64:
        case txt::type::p64:
        case txt::type::f64:   return 8;

        case txt::type::ivec2:
        case txt::type::vec2:  return 4 * 2;

        case txt::type::ivec3:
        case txt::type::vec3:  return 4 * 3;

        case txt::type::ivec4:
        case txt::type::vec4:  return 4 * 4;

        case txt::type::dvec2: return 8 * 2;
        case txt::type::dvec3: return 8 * 3;
        case txt::type::dvec4: return 8 * 4;

        case txt::type::mat2:  return 4 * 2 * 2;
        case txt::type::mat3:  return 4 * 3 * 3;
        case txt::type::mat4:  return 4 * 4 * 4;

        default: return 0;
    }
}

struct attribute_description {
    txt::type     format{txt::type::unknown};
    bool          normalized{false};
    std::uint32_t divisor{0};
};
using attribute_descriptions_t = std::vector<attribute_description>;

class vertex_buffer {
public:
    vertex_buffer(void const* data, std::size_t const& bytes, txt::type const& type, txt::usage const& usage, attribute_descriptions_t const& layout);
    ~vertex_buffer();

    auto bind()   const -> void;
    auto unbind() const -> void;

    auto id() const -> std::uint32_t { return m_id; }
    auto bytes() const -> std::size_t { return m_bytes; }
    auto type() const -> txt::type { return m_type; }
    auto usage() const -> txt::usage { return m_usage; }
    auto layout() const -> attribute_descriptions_t const& { return m_layout; }

    auto resize(std::size_t bytes) -> void;
    auto sub(void const* data, std::size_t bytes, std::size_t offset = 0) -> void;

private:
    std::uint32_t m_id;
    std::size_t   m_bytes;
    txt::type     m_type;
    txt::usage    m_usage;
    attribute_descriptions_t m_layout;
};

using vertex_buffer_ref_t = ref<vertex_buffer>;
auto make_vertex_buffer(void const* data, std::size_t const& bytes, txt::type const& type, txt::usage const& usage, attribute_descriptions_t const& layout) -> vertex_buffer_ref_t;

class index_buffer {
public:
    index_buffer(void const* data, std::size_t const& bytes, std::size_t const& size, txt::type const& type, txt::usage const& usage);
    ~index_buffer();

    auto bind()   const -> void;
    auto unbind() const -> void;

    auto id() const -> std::uint32_t { return m_id; }
    auto bytes() const -> std::size_t { return m_bytes; }
    auto size() const -> std::size_t { return m_size; }
    auto type() const -> txt::type { return m_type; }
    auto usage() const -> txt::usage { return m_usage; }

private:
    std::uint32_t m_id;
    std::size_t   m_bytes;
    std::size_t   m_size;
    txt::type     m_type;
    txt::usage    m_usage;
};

using index_buffer_ref_t = ref<index_buffer>;
auto make_index_buffer(void const* data, std::size_t const& bytes, std::size_t const& size, txt::type const& type, txt::usage const& usage) -> index_buffer_ref_t;

class attribute_descriptor {
public:
    attribute_descriptor();
    ~attribute_descriptor();

    auto add(vertex_buffer_ref_t buffer) -> void;
    auto bind()   const -> void;
    auto unbind() const -> void;

private:
    static auto compute_stride(attribute_descriptions_t const& layout) -> std::size_t;

private:
    std::uint32_t m_id;
    std::size_t   m_index;
    std::vector<vertex_buffer_ref_t> m_buffers{};
};

using attribute_descriptor_ref_t = ref<attribute_descriptor>;
auto make_attribute_descriptor() -> attribute_descriptor_ref_t;
} // namespace txt

#endif  // TXT_BUFFER_HPP
