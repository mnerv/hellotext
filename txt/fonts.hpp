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

// I tried to follow the analogy from Google Fonts: Family, type family or font family.
// https://fonts.google.com/knowledge/glossary/family_or_type_family_or_font_family

// Contains the rendered bitmap, belongs to typeface
struct glyph {
    std::uint32_t  codepoint{0};
    image_u8_ref_t bitmap{nullptr};
    glm::vec2      uv{0.0f};
};

// Contains the loaded font and rendered glyph, belongs to font family
class typeface {
public:

private:
    std::string m_filename;
    std::unordered_map<std::uint32_t, glyph> m_glyphs{};
};

// This will contain the texture atlas, belongs to font manager.
class font_family {
public:
    font_family();
    ~font_family();

private:
    std::vector<typeface> m_typefaces{};
};

using font_family_ref_t = ref<font_family>;

// Handle adding fonts and loading it. Do most of heavy lifting using freetype.
class font_manager {
public:
    font_manager();
    ~font_manager();

private:
    FT_Library m_library{};
    FT_Bitmap  m_bitmap{};
    std::vector<font_family_ref_t> m_font_families{};
};
} // namespace txt

#endif  // TXT_FONTS_HPP
