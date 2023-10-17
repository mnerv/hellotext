#include "fonts.hpp"
#include <filesystem>

namespace txt {
typeface::typeface(typeface_props const& props, font_family_weak_t const& font_family)
    : m_filename(props.filename)
    , m_family(font_family)
    , m_size(props.size)
    , m_mode(props.render_mode)
    , m_scale(props.scale) {
    load(props.ranges);
}

auto typeface::set_scale(double const& scale) -> void {
    m_scale = scale;
}
auto typeface::reload() -> void {
    // Check pointer expirations from weak ptr. We make sure that the object we have is still alive.
    if (m_family.expired()) throw std::runtime_error("Font family has expired!");
    auto const family = m_family.lock();
    if (family->manager().expired()) throw std::runtime_error("Font manager has expired!");
    auto const manager = family->manager().lock();
    auto const ft_library = manager->ft_library();
    auto const ft_bitmap  = manager->ft_bitmap();

    m_max_glyph_size = 0;
    for (auto const& [code, glyph] : m_glyphs)
        load_glyph(code, ft_library, ft_bitmap);
}
auto typeface::query(std::uint32_t const& code) -> glyph const& {
    auto it = m_glyphs.find(code);
    // FIXME: Try to load the glyph if not found
    if (it == std::end(m_glyphs)) {
        load_glyph(code);
        it = m_glyphs.find(code);
        if (it == std::end(m_glyphs)) return m_glyphs.at(' ');
    }
    return it->second;
}

auto typeface::load(character_range_t const& range) -> void {
    // Check pointer expirations from weak ptr. We make sure that the object we have is still alive.
    if (m_family.expired()) throw std::runtime_error("Font family has expired!");
    auto const family = m_family.lock();
    if (family->manager().expired()) throw std::runtime_error("Font manager has expired!");
    auto const manager = family->manager().lock();
    auto const ft_library = manager->ft_library();
    auto const ft_bitmap  = manager->ft_bitmap();

    m_channels = 1;
    m_flags    = FT_LOAD_RENDER;
    if (m_mode == text_render_mode::raster) {
        // Do nothing, we render as usual
    } else if (m_mode == text_render_mode::sdf) {
        m_flags |= FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    } else if (m_mode == text_render_mode::subpixel) {
        m_channels = 3;  // Set image channel to RGB for subpixel rendering.
        m_flags |= FT_LOAD_TARGET_(FT_RENDER_MODE_LCD);
        FT_Library_SetLcdFilter(manager->ft_library(), FT_LCD_FILTER_DEFAULT);
    }

    auto const ft_ec = FT_New_Face(ft_library, m_filename.c_str(), 0, &m_ft_face);
    if (ft_ec == FT_Err_Unknown_File_Format)
        throw std::runtime_error(fmt::format("Font file path '{}' have an unknown file format.", m_filename));
    else if(ft_ec)
        throw std::runtime_error(fmt::format("Error loading font '{}' file, error type not currently supported.", m_filename));
    else if (m_ft_face == nullptr)
        throw std::runtime_error("Error createing FT_Face!");

    FT_Set_Pixel_Sizes(m_ft_face, 0, std::uint32_t(std::round(double(m_size) * m_scale)));
    // Load initial character range
    for (std::uint32_t code = range[0]; code < range[1]; ++code)
        load_glyph(code, ft_library, ft_bitmap);
}

auto typeface::load_glyph(std::uint32_t const& code) -> void {
    // Check pointer expirations from weak ptr. We make sure that the object we have is still alive.
    if (m_family.expired()) throw std::runtime_error("Font family has expired!");
    auto const family = m_family.lock();
    if (family->manager().expired()) throw std::runtime_error("Font manager has expired!");
    auto const manager = family->manager().lock();
    auto const ft_library = manager->ft_library();
    auto const ft_bitmap  = manager->ft_bitmap();
    load_glyph(code, ft_library, ft_bitmap);
}
auto typeface::load_glyph(std::uint32_t const& code, FT_Library library, FT_Bitmap* bitmap) -> void {
    auto const index = FT_Get_Char_Index(m_ft_face, code);
    if (index == 0) return;
    if (FT_Load_Glyph(m_ft_face, index, m_flags)) return;

    auto const width     = m_ft_face->glyph->bitmap.width / std::uint32_t(m_channels);
    auto const height    = m_ft_face->glyph->bitmap.rows;
    auto const left      = m_ft_face->glyph->bitmap_left;
    auto const top       = m_ft_face->glyph->bitmap_top;
    auto const advance_x = m_ft_face->glyph->advance.x;
    auto const advance_y = m_ft_face->size->metrics.height;

    // Convert to one byte alignment
    FT_Bitmap_Convert(library, &m_ft_face->glyph->bitmap, bitmap, 1);
    auto img = make_image_u8(bitmap->buffer, width, height, m_channels);
    glyph new_glyph{
        .codepoint    = code,
        .bearing_left = left,
        .bearing_top  = top,
        .advance_x    = advance_x,
        .advance_y    = advance_y,
        .bitmap       = img,
        // .uv           = {0.0f, 0.0f},
    };
    m_glyphs[code] = new_glyph;

    auto const width_or_height = std::size_t(std::max(width, height));
    m_max_glyph_size = std::max(m_max_glyph_size, width_or_height);
}

font_family::font_family(std::string const& name, font_manager_weak_t const& font_manager)
    : m_name(name)
    , m_manager(font_manager) { }

auto font_family::reload() -> void {
    // TODO: Reload all the typefaces and unload bitmap of the stale ones.
}
auto font_family::add(typeface_props const& props) -> void {
    auto const style_name = props.style;
    auto const it = m_typefaces.find(style_name);
    if (it != std::end(m_typefaces)) {
        it->second->reload();
        return;  // Reload and return because style already exist!
    }
    auto tf = make_ref<txt::typeface>(props, shared_from_this());
    m_typefaces.insert({style_name, tf});
}
auto font_family::typeface(std::string const& style) const -> typeface_ref_t const& {
    auto const it = m_typefaces.find(style);
    if (it == std::end(m_typefaces))
        throw std::runtime_error(fmt::format("Typeface with style {} does not exist!", style));
    return it->second;
}

font_manager::font_manager() {
    if (FT_Init_FreeType(&m_library))
        throw std::runtime_error("Failed to initialise FreeType library");
    FT_Bitmap_Init(&m_bitmap);
}
font_manager::~font_manager() {
    FT_Bitmap_Done(m_library, &m_bitmap);
    FT_Done_FreeType(m_library);
}

auto font_manager::reload() -> void {
    // TODO: Go through all the font_families and its typefaces and reload the bitmaps and remove stale typefaces and its glyph.
}
auto font_manager::load(typeface_props const& props) -> void {
    if (!std::filesystem::exists(props.filename))
        throw std::runtime_error(fmt::format("Font file path '{}' does not exist!", props.filename));
    auto const family_name = props.family;
    auto const family_it = m_families.find(family_name);
    font_family_ref_t family = nullptr;
    if (family_it != std::end(m_families)) {
        family = family_it->second;
    } else {
        family = make_ref<font_family>(family_name, shared_from_this());
        m_families.insert({family_name, family});
    }
    family->add(props);
}
auto font_manager::family(std::string const& family_name) -> font_family_ref_t {
    // FIXME: Handle family name doesn't exist.
    auto it = m_families.find(family_name);
    if (it == std::end(m_families)) return nullptr;
    return it->second;
}
} // namespace txt
