#include "engine.hpp"

#include <cmath>

#include "renderer.hpp"
#include "utf8.h"

#include "graphics/image.hpp"

namespace txt {
[[maybe_unused]]static constexpr float quad_vertices[]{
//     x,     y,     z,       u,    v,
    0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
    1.0f,  1.0f,  0.0f,    1.0f, 1.0f,
    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
};
[[maybe_unused]]static constexpr std::uint32_t quad_cw_indices[]{
    0, 1, 2,
    0, 2, 3
};

text_engine::text_engine(window_ref_t window) : m_window(window) { }

auto text_engine::load(font_load_params const& params) -> void {
    m_manager.load(params);
    if (m_current == nullptr)
        set_default(std::begin(m_manager)->second);

    constexpr auto round_up2 = [](auto const& value) {
        return std::pow(2, std::ceil(std::log2(value) / std::log2(2)));
    };
    auto cols = static_cast<usize>(std::ceil(std::sqrt(m_current->size())));
    cols = static_cast<usize>(round_up2(cols));
    auto const size = static_cast<usize>(round_up2(m_current->max_font_size() * cols));
    m_buffer = make_image_u8(nullptr, size, size, m_current->color_channels());
    m_uv.y = size;

    for (auto const& [code, glyph] : m_current->data()) {
        insert_bitmap(glyph);
    }

    m_buffer->fliph();
    write_png("test.png", *m_buffer);
}

auto text_engine::draw(std::string const& str, glm::vec2 const& position, glm::vec4 const& color, glm::vec2 const& scale) -> void {
    (void)str;
    (void)position;
    (void)color;
    (void)scale;
}

auto text_engine::text_size(std::string const& str, glm::vec2 const& scale) -> glm::vec2 {
    (void)str;
    (void)scale;
    return {};
}

auto text_engine::text_size(std::string const& str, glm::vec2 const& scale, font const& font) -> glm::vec2 {
    (void)str;
    (void)scale;
    (void)font;
    return {};
}

auto text_engine::set_default(font& font) -> void {
    auto const& it = m_manager.find(font.name());
    if (it == std::end(m_manager.fonts()))
        throw std::runtime_error(fmt::format("Font: {} doesn't exist", font.name()));
    m_current = &font;
}

auto text_engine::generate() -> void {
}

auto text_engine::insert_bitmap(txt::glyph const& glyph) -> void {
    auto const& bm = glyph.bitmap;
    for (std::size_t i = 0; i < bm.height(); ++i) {
        for (std::size_t j = 0; j < bm.width(); ++j) {
            auto const pixel = bm.pixel(j, i);
            m_buffer->set(std::size_t(m_uv.x) + j, std::size_t(m_uv.y) - i, pixel);
        }
    }

    m_uv.x += std::int32_t(m_current->max_font_size());
    if (m_uv.x >= std::int32_t(m_buffer->width() - m_current->max_font_size())) {
        m_uv.x = 0;
        m_uv.y -= std::int32_t(m_current->max_font_size());
    }
}
} // namespace txt
