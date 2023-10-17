#include "text_engine.hpp"
#include "renderer.hpp"
#include "utf8.h"

namespace txt {
static constexpr float quad_vertices[]{
//     x,     y,     z,       u,    v,
    0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
    1.0f,  1.0f,  0.0f,    1.0f, 1.0f,
    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
};
static constexpr std::uint32_t quad_cw_indices[]{
    0, 1, 2,
    0, 2, 3
};

text_engine::text_engine(window_ref_t window, font_manager_ref_t manager) : m_window(window), m_manager(manager) {
    m_index_buffer = make_index_buffer(quad_cw_indices, sizeof(quad_cw_indices), len(quad_cw_indices), type::u32, usage::static_draw);
    m_instance_buffer = make_vertex_buffer(nullptr, sizeof(text_batch::gpu), type::f32, usage::dynamic_draw, {
        {type::vec4, false, 1},
        {type::vec3, false, 1},
        {type::vec3, false, 1},
        {type::vec2, false, 1},
        {type::vec2, false, 1},
    });
    m_descriptor = make_attribute_descriptor();
    m_descriptor->add(make_vertex_buffer(quad_vertices, sizeof(quad_vertices), type::f32, usage::static_draw, {
        {type::vec3, false, 0},
        {type::vec2, false, 0},
    }));
    m_descriptor->add(m_instance_buffer);

    m_manager->load({
        .filename    = "./res/fonts/Cozette/CozetteVector.ttf",
        .size        = 13,
        .family      = "Cozette",
        .style       = "Regular",
        .render_mode = text_render_mode::raster,
        .ranges      = default_character_range,
        .scale       = m_window->content_scale_x()
    });
    m_typeface = m_manager->family("Cozette")->typeface("Regular");
    reload();

#ifndef __EMSCRIPTEN__
    auto vs = read_text("./shaders/opengl/text.vert");
    auto fs = read_text("./shaders/opengl/text.frag");
#else
    auto vs = read_text("./shaders/webgl/text.vert");
    auto fs = read_text("./shaders/webgl/text.frag");
#endif
    m_shader_normal = make_shader(vs, fs);
}

auto text_engine::typeface(std::string const& family, std::string const& style) -> typeface_ref_t {
    auto const it = m_manager->families().find(family);
    if (it == std::end(m_manager->families())) return nullptr;
    return it->second->typeface(style);
}

auto text_engine::text(std::string const& txt, glm::vec3 const& position, glm::vec4 const& color, typeface_ref_t const& typeface, glm::vec3 const& scale) -> void {
    (void)txt;
    (void)position;
    (void)color;
    (void)typeface;
    (void)scale;
}
auto text_engine::text_size(std::string const& txt, typeface_ref_t const& typeface, glm::vec3 const& scale) const -> glm::vec2 {
    (void)txt;
    (void)typeface;
    (void)scale;
    return {};
}

auto text_engine::reload() -> void {
    // for (auto const& [name, family] : m_manager->families()) {
    //     for (auto const& [style, tf] : family->typefaces()) {
    //     }
    // }
}
auto text_engine::begin() -> void {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
auto text_engine::end() -> void {}
} // namespace txt
