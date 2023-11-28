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

font::font(FT_Face face, font_load_params const& params)
    : m_face(face)
    , m_filename(params.filename)
    , m_size(params.size)
    , m_render_mode(params.render_mode)
    , m_name(filename_no_extension(m_filename))
    , m_color_channels(1)
    , m_glyphs() { }

font::~font() {
    if (m_face == nullptr) return;
    FT_Done_Face(m_face);
}
font::font(font&& other) noexcept
    : m_face(std::exchange(other.m_face, nullptr))
    , m_filename(std::move(other.m_filename))
    , m_size(other.m_size)
    , m_render_mode(other.m_render_mode)
    , m_name(std::move(other.m_name)) { }

auto font::operator=(font&& other) noexcept -> font& {
    std::swap(m_face, other.m_face);
    m_filename = std::move(other.m_filename);
    std::swap(m_size, other.m_size);
    std::swap(m_render_mode, other.m_render_mode);
    m_name = std::move(other.m_name);
    return *this;
}

font_manager::font_manager()
    : m_library(nullptr)
    , m_bitmap(0)
    , m_fonts() {
    if (FT_Init_FreeType(&m_library))
        throw std::runtime_error("Failed to initialise FreeType library");
    FT_Bitmap_Init(&m_bitmap);
}
font_manager::~font_manager() {
    if (m_library == nullptr) return;  // FreeType library has been moved
    FT_Bitmap_Done(m_library, &m_bitmap);
    FT_Done_FreeType(m_library);
}
font_manager::font_manager(font_manager const& other)
    : m_library(nullptr)
    , m_bitmap(0)
    , m_fonts() {
    if (FT_Init_FreeType(&m_library))
        throw std::runtime_error("Failed to initialise FreeType library");
    FT_Bitmap_Init(&m_bitmap);
    (void)other;
}
auto font_manager::operator=(font_manager const& other) -> font_manager& {
    if (this == &other) return *this;
    return *this = font_manager(other);
}

auto font_manager::load(font_load_params const& params) -> void {
    if (!std::filesystem::exists(params.filename))
        throw std::runtime_error(fmt::format("Font file path '{}' does not exist!", params.filename));
    // TODO: Load font
}
auto font_manager::find(std::string const& name) const -> fonts_t::const_iterator {
    return std::ranges::find_if(m_fonts, [&name](auto const& font) { return font.name() == name; });
}
auto font_manager::erase(fonts_t::const_iterator const& it) -> void {
    m_fonts.erase(it);
}
} // namespace txt
