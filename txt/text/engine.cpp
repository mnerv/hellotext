#include "engine.hpp"

#include "renderer.hpp"
#include "utf8.h"

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
    set_default(std::begin(m_manager)->second);
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


auto text_engine::begin() -> void {
}

auto text_engine::end() -> void {
}

} // namespace txt
