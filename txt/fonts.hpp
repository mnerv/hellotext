#ifndef TXT_FONTS_HPP
#define TXT_FONTS_HPP
#include <cstdint>
#include <cstddef>
#include <set>
#include <vector>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_BITMAP_H

#include "glm/vec2.hpp"

#include "utility.hpp"
#include "image.hpp"

namespace txt {
enum class text_render_mode {
    normal,    // Gray scale anti-aliased
    sdf,       // (Signed Distance Field)
    subpixel,  // Using subpixel to anti-aliased
    raster     // For rasterised font, scaled with nearest neighbor
};

struct glyph;
class font;
class font_manager;

using font_manager_ref_t = ref<font_manager>;
constexpr std::uint32_t character_default_start_range = 32;
constexpr std::uint32_t character_default_end_range   = 127;
using character_range_t = std::array<std::uint32_t, 2>;
// Printable ASCII range 32..126 inclusive
[[maybe_unused]]constexpr character_range_t default_character_range{
    character_default_start_range,
    character_default_end_range
};

struct font_load_params {
    std::string       filename;
    std::uint32_t     size;
    text_render_mode  render_mode{text_render_mode::normal};
    character_range_t ranges{default_character_range};
};

struct glyph {
    std::uint32_t codepoint{0};
    std::int32_t  bearing_left{0};
    std::int32_t  bearing_top{0};
    std::int64_t  advance_x{0};
    std::int64_t  advance_y{0};
    image_u8      bitmap{};
};

class font {
public:
    ~font();
    font(font const& other) = delete;
    font(font&& other) noexcept;
    auto operator=(font const& other) -> font& = delete;
    auto operator=(font&& other) noexcept -> font&;

    auto filename() const -> std::string const& { return m_filename; }
    auto size() const -> std::uint32_t { return m_size; }
    auto render_mode() const -> text_render_mode { return m_render_mode; }
    auto name() const -> std::string const& { return m_name; }
    auto color_channels() const -> std::size_t { return m_color_channels; }
    auto find(std::uint32_t const& code) const -> glyph const&;

private:
    friend font_manager;
    font(FT_Face face, font_load_params const& params);

private:
    FT_Face          m_face;
    std::string      m_filename;
    std::uint32_t    m_size;
    text_render_mode m_render_mode;
    std::string      m_name;
    std::size_t      m_color_channels;
    std::unordered_map<std::uint32_t, glyph> m_glyphs;
};

class font_manager {
public:
    using fonts_t = std::vector<txt::font>;

public:
    font_manager();
    ~font_manager();
    font_manager(font_manager const& other);
    // font_manager(font_manager&& other) noexcept;
    auto operator=(font_manager const& other) -> font_manager&;
    // auto operator=(font_manager&& other) noexcept -> font_manager&;

    auto fonts() const -> fonts_t const& { return m_fonts; }
    auto load(font_load_params const& params) -> void;
    auto find(std::string const& name) const -> fonts_t::const_iterator;
    auto erase(fonts_t::const_iterator const& it) -> void;

public:
    auto begin() -> fonts_t::iterator { return std::begin(m_fonts); }
    auto end() -> fonts_t::iterator { return std::begin(m_fonts); }
    auto begin() const -> fonts_t::const_iterator { return std::begin(m_fonts); }
    auto end() const -> fonts_t::const_iterator { return std::begin(m_fonts); }

private:
    FT_Library m_library;
    FT_Bitmap  m_bitmap;
    fonts_t    m_fonts;
};
} // namespace txt

#endif  // TXT_FONTS_HPP
