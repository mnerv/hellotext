#ifndef TXT_FONTS_HPP
#define TXT_FONTS_HPP
#include <cstdint>
#include <cstddef>
#include <set>

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

// Forward declaration
struct glyph;
class typeface;
class font_family;
class font_manager;

// Alias ref pointer
using typeface_ref_t      = ref<typeface>;
using font_family_ref_t   = ref<font_family>;
using font_manager_ref_t  = ref<font_manager>;

using font_family_weak_t  = weak<font_family>;
using font_manager_weak_t = weak<font_manager>;

using character_range_t = std::array<std::uint32_t, 2>;

constexpr character_range_t default_character_range{32, 127};  // Printable ASCII range 32..126 inclusive

// I tried to follow the analogy from Google Fonts: Family, type family or font family.
// https://fonts.google.com/knowledge/glossary/family_or_type_family_or_font_family

// Contains the rendered bitmap, belongs to typeface
struct glyph {
    std::uint32_t  codepoint{0};
    std::int32_t   bearing_left{0};
    std::int32_t   bearing_top{0};
    std::int64_t   advance_x{0};
    std::int64_t   advance_y{0};
    image_u8_ref_t bitmap{nullptr};
};

struct typeface_props {
    std::string       filename;
    std::uint32_t     size;
    std::string       family;
    std::string       style;
    text_render_mode  render_mode{text_render_mode::normal};
    character_range_t ranges{default_character_range};
    double            scale{1.0};
};

// Contains the loaded font and rendered glyph, belongs to font family
class typeface : public std::enable_shared_from_this<typeface> {
public:
    using glyph_map_t = std::unordered_map<std::uint32_t, glyph>;

public:
    typeface(typeface_props const& props, font_family_weak_t const& font_family);
    ~typeface() = default;

    auto filename() const -> std::string const& { return m_filename; }
    auto font_size() const -> std::uint32_t { return m_font_size; }
    auto font_scale() const -> double { return m_font_scale; }
    auto render_mode() const -> text_render_mode { return m_mode; }
    auto size() const -> std::size_t { return m_glyphs.size(); }
    auto data() const -> glyph_map_t const& { return m_glyphs; }
    auto channels() const -> std::size_t { return m_atlas_channels; }
    auto family_name() const -> std::string const& { return m_family_name; }
    auto max_size_dim() const -> std::size_t { return m_max_size_dim; }

    auto set_font_size(std::uint32_t const& size) -> void;
    auto set_font_scale(double const& scale) -> void;
    auto set_mode(text_render_mode const& mode) -> void;

    auto reload() -> void;
    auto find(std::uint32_t const& code) const -> glyph_map_t::const_iterator { return m_glyphs.find(code); }
    auto load(std::uint32_t const& code) -> glyph_map_t::const_iterator;
    auto begin() const -> glyph_map_t::const_iterator { return std::begin(m_glyphs); }
    auto end() const -> glyph_map_t::const_iterator { return std::end(m_glyphs); }

private:
    [[nodiscard]]auto retrieve_ft() -> std::pair<FT_Library, FT_Bitmap*>;
    auto init_rendering_mode(FT_Library library) -> void;
    auto load(character_range_t const& range = default_character_range) -> void;
    auto load_glyph(std::uint32_t const& code) -> void;
    auto load_glyph(std::uint32_t const& code, FT_Library library, FT_Bitmap* bitmap) -> void;

private:
    std::string        m_filename;
    font_family_weak_t m_family;
    std::uint32_t      m_font_size;
    double             m_font_scale;
    text_render_mode   m_mode;
    std::string        m_family_name;
    std::int32_t       m_ft_flags{0x00'00'00'00};
    std::size_t        m_atlas_channels{0};
    FT_Face            m_ft_face{nullptr};
    glyph_map_t        m_glyphs{};
    std::size_t        m_max_size_dim{0};
};

class font_family : public std::enable_shared_from_this<font_family> {
public:
    font_family(std::string const& name, font_manager_weak_t const& font_manager);
    ~font_family() = default;

    auto name() const -> std::string const& { return m_name; }
    auto typefaces() const -> std::unordered_map<std::string, typeface_ref_t> const& { return m_typefaces; }
    auto reload() -> void;
    auto add(typeface_props const& props) -> void;
    auto typeface(std::string const& style) const -> typeface_ref_t const&;

private:
    friend txt::typeface;
    auto manager() -> font_manager_weak_t { return m_manager; }

private:
    std::string m_name;
    font_manager_weak_t m_manager;
    std::unordered_map<std::string, typeface_ref_t> m_typefaces{};
};

// Handle adding fonts and loading it. Do most of heavy lifting using freetype.
class font_manager : public std::enable_shared_from_this<font_manager> {
public:
    font_manager();
    ~font_manager();

    auto families() const -> std::unordered_map<std::string, font_family_ref_t> const& { return m_families; }
    auto reload() -> void;
    auto load(typeface_props const& props) -> void;
    auto family(std::string const& family_name) -> font_family_ref_t;

private:
    friend typeface;
    auto ft_library() -> FT_Library { return m_library; }
    auto ft_bitmap() -> FT_Bitmap* { return &m_bitmap; }

private:
    FT_Library m_library{};
    FT_Bitmap  m_bitmap{};
    std::unordered_map<std::string, font_family_ref_t> m_families;
};

} // namespace txt

#endif  // TXT_FONTS_HPP
