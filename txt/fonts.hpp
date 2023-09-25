#ifndef TXT_FONTS_HPP
#define TXT_FONTS_HPP
#include <cstdint>
#include <cstddef>
#include <set>

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_BITMAP_H

#include "image.hpp"

namespace txt {
enum class text_render_mode {
    normal,    // Gray scale anti-aliased
    sdf,       // (Signed Distance Field)
    subpixel,  // Using subpixel to anti-aliased
    raster     // For rasterised font, scaled with nearest neighbor
};
using atlas_ref_t = image_rgba;

class typeface {};

class font_family {};

class font_manager {};
} // namespace txt

#endif  // TXT_FONTS_HPP
