#ifndef TXT_DESCRIPTOR_HPP
#define TXT_DESCRIPTOR_HPP
#include <cstdint>
#include <cstddef>
#include <initializer_list>
#include <vector>
#include <memory>

namespace txt {
// OpenGL Type: https://www.khronos.org/opengl/wiki/OpenGL_Type
enum class format : std::uint32_t {
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

struct attribute_description {
    std::uint32_t location{0};
    txt::format   format{txt::format::unknown};
    std::uint32_t binding{0};
    std::uint32_t offset{0};
};
using attribute_descriptions_t = std::vector<attribute_description>;

class layout_descriptor {
public:
    layout_descriptor();
    layout_descriptor(std::initializer_list<attribute_description> const& descriptions);
    ~layout_descriptor();

    auto init(std::initializer_list<attribute_description> const& descriptions) -> void;
    auto bind() -> void;
    auto unbind() -> void;
    auto bytes() const -> std::size_t { return m_bytes; }
    auto layout() const -> attribute_descriptions_t const& { return m_layout; }

private:
    auto clean_up() -> void;

private:
    std::uint32_t m_id{0};
    attribute_descriptions_t m_layout{};
    std::size_t m_bytes{0};
};

using layout_descriptor_ref_t = std::shared_ptr<layout_descriptor>;
auto make_layout_descriptor(std::initializer_list<attribute_description> const& descriptions) -> layout_descriptor_ref_t;
} // namespace txt

#endif  // TXT_DESCRIPTOR_HPP
