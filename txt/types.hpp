#ifndef TXT_TYPES_HPP
#define TXT_TYPES_HPP

#include <cstdint>
#include <cstddef>

namespace txt {
namespace types {
    using f32 = float;
    using f64 = double;

    using u8 = std::uint8_t;
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using usize = std::size_t;
    using isize = std::ptrdiff_t;

    using i8 = std::int8_t;
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using c8 = char;
    using c16 = char16_t;
    using c32 = char32_t;
}

using namespace txt::types;

// OpenGL Type: https://www.khronos.org/opengl/wiki/OpenGL_Type
enum class type : u32 {
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

enum class usage : u32 {
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
}

#endif  // TXT_TYPES_HPP
