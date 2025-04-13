#include "fonts.hpp"
#include <filesystem>
#include <algorithm>

namespace txt {
[[maybe_unused]]static auto filename_no_extension(std::string const& filename) -> std::string {
    auto const path = std::filesystem::path(filename);
    auto name = path.filename().string();
    auto const it = name.find(path.extension().string());
    if (it != std::string::npos) name.erase(it);
    return name;
}

font::font()
    : m_face()
    , m_filepath("")
    , m_size(0)
    , m_render_mode(text_render_mode::normal)
    , m_name("") {}

font::font(FT_Face face, font_load_params const& params)
    : m_face(face)
    , m_filepath(params.filepath)
    , m_size(params.size)
    , m_render_mode(params.render_mode)
    , m_name(filename_no_extension(m_filepath)) {
}

font::~font() {
    if (m_face == nullptr) return;
    FT_Done_Face(m_face);
}

font::font(font&& other) noexcept
    : m_face(std::exchange(other.m_face, nullptr))
    , m_filepath(std::move(other.m_filepath))
    , m_size(other.m_size)
    , m_render_mode(other.m_render_mode)
    , m_name(std::move(other.m_name)) { }

auto font::operator=(font&& other) noexcept -> font& {
    std::swap(m_face, other.m_face);
    m_filepath = std::move(other.m_filepath);
    std::swap(m_size, other.m_size);
    std::swap(m_render_mode, other.m_render_mode);
    m_name = std::move(other.m_name);
    return *this;
}

auto font::find(std::uint32_t const& code) -> glyph_it {
    return m_glyphs.find(code);
}

auto font::load_glyph(std::uint32_t const& code, FT_Library library, FT_Bitmap* bitmap) -> void {
    auto const index = FT_Get_Char_Index(m_face, code);
    if (index == 0) return;
    if (FT_Load_Glyph(m_face, index, m_flags)) return;

    auto const width     = m_face->glyph->bitmap.width / static_cast<std::uint32_t>(m_color_channels);
    auto const height    = m_face->glyph->bitmap.rows;
    auto const left      = m_face->glyph->bitmap_left;
    auto const top       = m_face->glyph->bitmap_top;
    auto const advance_x = m_face->glyph->advance.x;
    auto const advance_y = m_face->size->metrics.height;

    // Convert to one byte alignment
    FT_Bitmap_Convert(library, &m_face->glyph->bitmap, bitmap, 1);
    m_glyphs.insert_or_assign(code, glyph{
        .codepoint    = code,
        .bearing_left = left,
        .bearing_top  = top,
        .advance_x    = advance_x,
        .advance_y    = advance_y,
        .bitmap       = image_u8(bitmap->buffer, width, height, m_color_channels),
    });
}

font_manager::font_manager()
    : m_library(nullptr)
    , m_bitmap()
    , m_fonts() {
    if (FT_Init_FreeType(&m_library))
        throw std::runtime_error("Failed to initialise FreeType library");
    FT_Bitmap_Init(&m_bitmap);
}
font_manager::~font_manager() {
    if (m_library == nullptr) return;  // FreeType library has been moved
    FT_Bitmap_Done(m_library, &m_bitmap);
    m_fonts.clear();
    FT_Done_FreeType(m_library);
}

auto font_manager::load(font& font, std::uint32_t const& code) -> void {
    auto const& it = m_fonts.find(font.name());
    if (it == std::end(m_fonts)) return;

    auto const index = FT_Get_Char_Index(font.m_face, code);
    if (index == 0) return;

    auto& face   = font.m_face;
    auto& flags  = font.m_flags;
    auto& glyphs = font.m_glyphs;

    if (FT_Load_Glyph(face, index, flags)) return;

    auto const width     = font.m_face->glyph->bitmap.width / static_cast<std::uint32_t>(font.m_color_channels);
    auto const height    = font.m_face->glyph->bitmap.rows;
    auto const left      = font.m_face->glyph->bitmap_left;
    auto const top       = font.m_face->glyph->bitmap_top;
    auto const advance_x = font.m_face->glyph->advance.x;
    auto const advance_y = font.m_face->size->metrics.height;

    // Convert to one byte alignment
    FT_Bitmap_Convert(m_library, &face->glyph->bitmap, &m_bitmap, 1);
    glyphs.insert_or_assign(code, glyph{
        .codepoint    = code,
        .bearing_left = left,
        .bearing_top  = top,
        .advance_x    = advance_x,
        .advance_y    = advance_y,
        .bitmap       = image_u8(m_bitmap.buffer, width, height, font.m_color_channels),
    });
}

auto font_manager::load(font_load_params const& params, std::string const& name) -> void {
    auto const filepath = std::filesystem::path(params.filepath);
    if (!std::filesystem::exists(filepath))
        throw std::runtime_error(fmt::format("Font file path '{}' does not exist!", params.filepath));
    if (std::filesystem::is_directory(filepath))
        throw std::runtime_error(fmt::format("Path '{}' given is directory", params.filepath));
    auto const filename_no_ext = [&]() {
        auto str = filepath.filename().string();
        str.erase(str.find(filepath.extension().string()));
        return str;
    };
    auto const font_name = name.empty() ? filename_no_ext() : name;

    FT_Face face{};
    auto const ft_ec = FT_New_Face(m_library, params.filepath.c_str() , 0, &face);
    if (ft_ec == FT_Err_Unknown_File_Format)
        throw std::runtime_error(fmt::format("Font file path '{}' have an unknown file format.", params.filepath));
    else if(ft_ec)
        throw std::runtime_error(fmt::format("Error loading font '{}' file, error type not currently supported.", params.filepath));
    else if (face == nullptr)
        throw std::runtime_error("Error creating FT_Face!");

    font font{face, params};
    font.m_color_channels = 1;
    font.m_flags          = FT_LOAD_RENDER;
    if (font.m_render_mode == text_render_mode::raster) {
        // Do nothing, we render as usual
    } else if (font.m_render_mode == text_render_mode::sdf) {
        font.m_flags |= FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    } else if (font.m_render_mode == text_render_mode::subpixel) {
        font.m_color_channels = 3;  // Set image channel to RGB for subpixel rendering.
        font.m_flags |= FT_LOAD_TARGET_(FT_RENDER_MODE_LCD);
        FT_Library_SetLcdFilter(m_library, FT_LCD_FILTER_DEFAULT);
    }

    auto const& range = params.ranges;
    FT_Set_Pixel_Sizes(font.m_face, 0, std::uint32_t(font.m_size));
    // Load initial character range
    for (std::uint32_t code = range[0]; code < range[1]; ++code)
        load(font, code);

    m_fonts.insert({font_name, std::move(font)});
}
auto font_manager::find(std::string const& name) const -> fonts_t::const_iterator {
    return m_fonts.find(name);
}
auto font_manager::erase(fonts_t::const_iterator const& it) -> void {
    m_fonts.erase(it);
}
} // namespace txt
