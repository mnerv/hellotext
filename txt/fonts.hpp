#ifndef TXT_FONTS_HPP
#define TXT_FONTS_HPP
#include <cstdint>
#include <cstddef>
#include <set>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_BITMAP_H

#include "utility.hpp"
#include "image.hpp"

namespace txt {
enum class text_render_mode {
    normal,    // Gray scale anti-aliased
    sdf,       // (Signed Distance Field)
    subpixel,  // Using subpixel to anti-aliased
    raster     // For rasterised font, scaled with nearest neighbor
};
using atlas_ref_t = ref<image_u8>;

// I tried to follow the analogy from Google Fonts: Family, type family or font family.
// https://fonts.google.com/knowledge/glossary/family_or_type_family_or_font_family

// Contains the rendered bitmap, belongs to typeface
struct glyph {
};

// Contains the loaded font and rendered glyph, belongs to font family
class typeface {
};

// This will contain the texture atlas, belongs to font manager.
class font_family {
};

// Handle adding fonts and loading it. Do most of heavy lifting using freetype.
class font_manager {
};
} // namespace txt

#endif  // TXT_FONTS_HPP
