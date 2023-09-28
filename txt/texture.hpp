#ifndef TXT_TEXTURE_HPP
#define TXT_TEXTURE_HPP
#include <cstddef>
#include <cstdint>
#include "image.hpp"

namespace txt {
enum class pixel_fmt : std::uint32_t {
    unknown = 0,
    red,
    rg,
    rgb,
    bgr,
    rgba,
    bgra,
    red_integer,
    rg_integer,
    rgb_integer,
    bgr_integer,
    rgba_integer,
    bgra_integer,
    stencil_index,
    depth_component,
    depth_stencil,
};

enum class pixel_type : std::uint32_t {
    unknown = 0,
    u8,             i8,
    u16,            i16,
    u32,            i32,
    f16,            f32,
    u8_3_3_2,       u8_2_3_3_rev,
    u16_5_6_5,      u16_5_6_5_rev,
    u16_4_4_4_4,    u16_4_4_4_4_rev,
    u16_5_5_5_1,    u16_1_5_5_5_rev,
    u32_8_8_8_8,    u32_8_8_8_8_rev,
    u32_10_10_10_2, u32_2_10_10_10_rev
};

struct texture_props {
    pixel_fmt internal{pixel_fmt::rgba};
    pixel_fmt format{pixel_fmt::rgba};
};

class texture {
public:
    texture(void const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels, texture_props const& props = {});
    ~texture();

    auto width() const -> std::size_t { return m_width; }
    auto height() const -> std::size_t { return m_height; }
    auto channels() const -> std::size_t { return m_channels; }

    auto bind(std::size_t const& slot = 0) const -> void;
    auto unbind(std::size_t const& slot = 0) const -> void;

private:
    std::uint32_t m_id;
    std::size_t   m_width;
    std::size_t   m_height;
    std::size_t   m_channels;
};
}

#endif  // TXT_TEXTURE_HPP
