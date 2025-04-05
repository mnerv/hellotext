#include "renderer.hpp"
#include <stdexcept>
#include "fmt/format.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace txt {
[[maybe_unused]]static constexpr float QUAD_VERTICES[]{
//     x,    y,    z,     u,   v
    -0.5, -0.5,  0.0,   0.0, 0.0,
    -0.5,  0.5,  0.0,   0.0, 1.0,
     0.5,  0.5,  0.0,   1.0, 1.0,
     0.5, -0.5,  0.0,   1.0, 0.0,
};
[[maybe_unused]]static constexpr std::uint32_t QUAD_INDICES_CW[]{
    0, 1, 2,
    0, 2, 3,
};

auto hsb2rgb(float hue, float saturation, float brightness) -> glm::vec3 {
    auto const h = hue / 360.0f;
    auto rgb = glm::clamp(glm::abs(glm::mod(h * 6.0f + glm::vec3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f);
    rgb = rgb * rgb * (3.0f - 2.0f * rgb);
    return brightness * glm::mix(glm::vec3(1.0f), rgb, saturation);
}


auto make_renderer(window_ref_t window) -> renderer_local_t {
    if (!window->is_init()) window->setup();
    return std::make_unique<renderer>(window);
}

renderer::renderer(window_ref_t window) : m_window(window) {
    m_rect_default_shader = make_shader(
        read_text("./shaders/opengl/base.vert"),
        read_text("./shaders/opengl/color.frag")
    );
    m_rect_texture_shader = make_shader(
        read_text("./shaders/opengl/base.vert"),
        read_text("./shaders/opengl/texture.frag")
    );
    m_rect_index_buffer = make_index_buffer(QUAD_INDICES_CW,
                                            sizeof(QUAD_INDICES_CW),
                                            len(QUAD_INDICES_CW),
                                            type::u32, usage::static_draw);
    m_rect_vertex_buffer = make_vertex_buffer(QUAD_VERTICES,
                                              sizeof(QUAD_VERTICES), type::f32,
                                              usage::dynamic_draw, {
        {type::vec4, false, 1},
        {type::vec3, false, 1},
        {type::vec3, false, 1},
        {type::vec3, false, 1},
        {type::vec2, false, 1},
        {type::vec2, false, 1}
    });
    m_rect_descriptor = make_attribute_descriptor();
    m_rect_descriptor->add(make_vertex_buffer(QUAD_VERTICES,
                                              sizeof(QUAD_VERTICES), type::f32,
                                              usage::static_draw, {
        {type::vec3, false, 0},
        {type::vec2, false, 0},
    }));
    m_rect_descriptor->add(m_rect_vertex_buffer);

    m_text_engine = make_ref<text_engine>(m_window);
}

renderer::~renderer() { }

auto renderer::load_font(font_load_params const& params) -> void {
    m_text_engine->load(params);
}

auto renderer::begin() -> void {
    m_view = glm::lookAt(glm::vec3{0.0, 0.0, 1023.0}, glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0});
    m_projection = glm::ortho(0.0f, float(m_window->width()), 0.0f, float(m_window->height()), 0.1f, 1024.0f);

    m_color_rect_size = 0;
    for (auto& [st, data] : m_shader_texture_rects)
        st.size = 0;

    m_text_engine->begin();
}

auto renderer::end() -> void {
    if (m_color_rect_size > 0) {
        auto const bytes = m_color_rect_size * sizeof(rect_instance);
        m_rect_vertex_buffer->bind();
        m_rect_vertex_buffer->resize(bytes);
        m_rect_vertex_buffer->sub(m_color_rects.data(), bytes, 0);

        m_rect_default_shader->bind();
        m_rect_default_shader->upload_mat4("u_model", m_model);
        m_rect_default_shader->upload_mat4("u_view", m_view);
        m_rect_default_shader->upload_mat4("u_projection", m_projection);
        m_rect_descriptor->bind();
        m_rect_index_buffer->bind();
        glDrawElementsInstanced(GL_TRIANGLES, GLsizei(m_rect_index_buffer->size()), gl_type(m_rect_index_buffer->type()), nullptr, GLsizei(m_color_rect_size));
    }
    for (auto const& [st, vec] : m_shader_texture_rects) {
        auto const bytes = st.size * sizeof(rect_instance);
        m_rect_vertex_buffer->bind();
        m_rect_vertex_buffer->resize(bytes);
        m_rect_vertex_buffer->sub(vec.data(), bytes, 0);

        st.shader->bind();
        st.shader->upload_mat4("u_model", m_model);
        st.shader->upload_mat4("u_view", m_view);
        st.shader->upload_mat4("u_projection", m_projection);
        st.shader->upload_num("u_texture", 0.0f);
        st.texture->bind(0);
        m_rect_descriptor->bind();
        m_rect_index_buffer->bind();
        glDrawElementsInstanced(GL_TRIANGLES, GLsizei(m_rect_index_buffer->size()), gl_type(m_rect_index_buffer->type()), nullptr, GLsizei(m_color_rect_size));
    }
    m_text_engine->end();
}

auto renderer::viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) -> void {
    glViewport(x, y, GLsizei(width), GLsizei(height));
}

auto renderer::clear_color(std::uint32_t color, float alpha) -> void {
    auto const r = float((color >> 16) & 0xFF) / 255.0f;
    auto const g = float((color >>  8) & 0xFF) / 255.0f;
    auto const b = float((color >>  0) & 0xFF) / 255.0f;
    glClearColor(r, g, b, alpha);
}

auto renderer::clear(GLenum bitmask) -> void {
    glClear(bitmask);
}

auto renderer::rect(glm::vec2 const& position, glm::vec2 const& size,
                    float const& rotation, glm::vec4 const& color,
                    [[maybe_unused]]glm::vec4 const& round) -> void {
    rect_instance rect{
        .color     = color,
        .position  = {position, m_depth},
        .scale     = {size, 1.0f},
        .rotation  = {0.0f, 0.0f, rotation},
        .uv_offset = {0.0f, 0.0f},
        .uv_size   = {1.0f, 1.0f}
    };

    if (m_color_rect_size < m_color_rects.size()) {
        m_color_rects[m_color_rect_size] = rect;
    } else {
        m_color_rects.push_back(rect);
    }
    ++m_color_rect_size;
    m_depth += m_depth_step;
}

auto renderer::rect(glm::vec2 const& position, glm::vec2 const& size,
                    float const& rotation, shader_ref_t shader,
                    texture_ref_t texture, glm::vec2 const& uv,
                    glm::vec2 const& uv_size, float const& z_offset) -> void {
    rect_instance rect{
        .color     = {1.0f, 1.0f, 1.0f, 1.0f},
        .position  = {position, m_depth + z_offset},
        .scale     = {size, 1.0f},
        .rotation  = {0.0f, 0.0f, rotation},
        .uv_offset = uv,
        .uv_size   = uv_size
    };
    shader_texture_pair key{shader, texture};
    auto it = m_shader_texture_rects.find(key);
    if (it == std::end(m_shader_texture_rects)) {
        m_shader_texture_rects[key] = {};
        it = m_shader_texture_rects.find(key);
    }
    it->second.push_back(rect);
    ++it->first.size;
}

auto renderer::text(std::string const& str, glm::vec2 const& position,
                    glm::vec4 const& color, glm::vec2 const& scale) -> void {
    m_text_engine->draw(str, position, color, scale);
    m_depth += m_depth_step;
}

auto renderer::text_size(std::string const& str,
                         glm::vec2 const& scale) -> glm::vec2 {
    return m_text_engine->text_size(str, scale);
}

auto renderer::zdepth() const -> float { return m_depth; }
} // namespace txt
