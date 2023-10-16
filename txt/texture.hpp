#ifndef TXT_TEXTURE_HPP
#define TXT_TEXTURE_HPP
#include <cstddef>
#include <cstdint>

#include "utility.hpp"
#include "buffer.hpp"
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

enum class tex_filter : std::uint32_t {
    unknown = 0,
    nearest,
    linear,
    nearest_mipmap_nearest,
    linear_mipmap_nearest,
    nearest_mipmap_linear,
    linear_mipmap_linear,
};

enum class tex_wrap : std::uint32_t {
    unknown = 0,
    repeat,
    mirrored_repeat,
    clamp_to_edge,
    clamp_to_border,
    mirror_clamp_to_edge
};

struct texture_props {
    pixel_fmt  internal{pixel_fmt::rgba};
    pixel_fmt  format{pixel_fmt::rgba};          // This will be infered if image is passed in.
    tex_filter min_filter{tex_filter::nearest};
    tex_filter mag_filter{tex_filter::nearest};
    tex_wrap   wrap_s{tex_wrap::clamp_to_edge};  // x
    tex_wrap   wrap_t{tex_wrap::clamp_to_edge};  // y
    tex_wrap   wrap_r{tex_wrap::clamp_to_edge};  // z - only if you're using 3D texture
    bool       mipmap{true};
    txt::type  data_type{txt::type::u8};
};

class texture {
public:
    texture(void const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels, texture_props const& props = {});
    ~texture();

    auto id() const -> std::uint32_t { return m_id; }
    auto width() const -> std::size_t { return m_width; }
    auto height() const -> std::size_t { return m_height; }

    auto set(void const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels, texture_props const& props = {}) -> void;
    auto set(image_u8_ref_t img, texture_props const& props = {}) -> void;
    auto bind(std::size_t const& slot = 0) const -> void;
    auto unbind(std::size_t const& slot = 0) const -> void;

private:
    std::uint32_t m_id;
    std::size_t   m_width;
    std::size_t   m_height;
    std::size_t   m_channels;
};

using texture_ref_t = ref<texture>;
auto make_texture(void const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels, texture_props const& props) -> texture_ref_t;
auto make_texture(image_u8_ref_t img, texture_props const& props = {}) -> texture_ref_t;
}

#endif  // TXT_TEXTURE_HPP
