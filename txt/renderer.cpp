#include "renderer.hpp"
#include <stdexcept>
#include "fmt/format.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace txt {
static renderer::local_t s_instance = nullptr;
static constexpr float QUAD_VERTICES[]{
//     x,    y,    z,     u,   v
    -0.5, -0.5,  0.0,   0.0, 0.0,
    -0.5,  0.5,  0.0,   0.0, 1.0,
     0.5,  0.5,  0.0,   1.0, 1.0,
     0.5, -0.5,  0.0,   1.0, 0.0,
};
static constexpr std::uint32_t QUAD_INDICES_CW[]{
    0, 1, 2,
    0, 2, 3,
};

auto renderer::init(window_ref_t window) -> void {
    if (s_instance != nullptr) throw std::runtime_error("txt::render has already been initialised!");
    s_instance = std::make_unique<renderer>(window);
}
auto renderer::instance() -> local_t& {
    return s_instance;
}

auto begin_frame() -> void {
    s_instance->begin();
}
auto end_frame() -> void {
    s_instance->end();
}
auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) -> void {
    renderer::viewport(x, y, width, height);
}
auto clear_color(std::uint32_t color, float alpha) -> void {
    renderer::clear_color(color, alpha);
}
auto clear(GLenum bitmask) -> void {
    renderer::clear(bitmask);
}
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color, glm::vec4 const& round) -> void {
    s_instance->rect(position, size, rotation, color, round);
}
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, texture_ref_t texture, glm::vec2 const& uv, glm::vec2 const& uv_size, glm::vec4 const& round) -> void {
    s_instance->rect(position, size, rotation, texture, uv, uv_size, round);
}
auto text(std::string const& str, glm::vec2 const& position, glm::vec4 const& color, glm::vec2 const& scale, typeface_ref_t const& tf) -> void {
    s_instance->text(str, position, color, scale, tf);
}
auto text_size(std::string const& str, glm::vec2 const& scale, typeface_ref_t const& typeface) -> glm::vec2 {
    return s_instance->text_size(str, scale, typeface);
}

renderer::renderer(window_ref_t window) : m_window(window) {
#ifndef __EMSCRIPTEN__
    m_rect_default_shader = make_shader(
        read_text("./shaders/opengl/base.vert"),
        read_text("./shaders/opengl/color.frag")
    );
    m_rect_texture_shader = make_shader(
        read_text("./shaders/opengl/base.vert"),
        read_text("./shaders/opengl/texture.frag")
    );
#else
    m_rect_default_shader = make_shader(
        read_text("./shaders/webgl/base.vert"),
        read_text("./shaders/webgl/color.frag")
    );
    m_rect_texture_shader = make_shader(
        read_text("./shaders/webgl/base.vert"),
        read_text("./shaders/webgl/texture.frag")
    );
#endif
    m_rect_index_buffer = make_index_buffer(QUAD_INDICES_CW, sizeof(QUAD_INDICES_CW), len(QUAD_INDICES_CW), type::u32, usage::static_draw);
    m_rect_vertex_buffer = make_vertex_buffer(QUAD_VERTICES, sizeof(QUAD_VERTICES), type::f32, usage::dynamic_draw, {
        {type::vec4, false, 1},
        {type::vec3, false, 1},
        {type::vec3, false, 1},
        {type::vec3, false, 1},
        {type::vec2, false, 1},
        {type::vec2, false, 1}
    });
    m_rect_descriptor = make_attribute_descriptor();
    m_rect_descriptor->add(make_vertex_buffer(QUAD_VERTICES, sizeof(QUAD_VERTICES), type::f32, usage::static_draw, {
        {type::vec3, false, 0},
        {type::vec2, false, 0},
    }));
    m_rect_descriptor->add(m_rect_vertex_buffer);

    m_text_engine = make_ref<txt::text_engine>(m_window, make_ref<font_manager>());
}
renderer::~renderer() { }
auto renderer::begin() -> void {
    m_view = glm::lookAt(glm::vec3{0.0, 0.0, 1023.0}, glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0});
    m_projection = glm::ortho(0.0f, float(m_window->width()), 0.0f, float(m_window->height()), 0.1f, 1024.0f);

    m_color_rect_size = 0;
    for (auto& [st, data] : m_shader_texture_rects)
        st.size = 0;
    m_text_engine->begin();
    m_depth = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
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
    m_text_engine->set_camera(m_view, m_projection);
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

auto renderer::rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color, [[maybe_unused]]glm::vec4 const& round) -> void {
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

auto renderer::rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, texture_ref_t texture, glm::vec2 const& uv, glm::vec2 const& uv_size, [[maybe_unused]]glm::vec4 const& round) -> void {
    rect(position, size, rotation, m_rect_texture_shader, texture, uv, uv_size, round);
}

auto renderer::rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, shader_ref_t shader, texture_ref_t texture, glm::vec2 const& uv, glm::vec2 const& uv_size, [[maybe_unused]]glm::vec4 const& round) -> void {
    rect_instance rect{
        .color     = {1.0f, 1.0f, 1.0f, 1.0f},
        .position  = {position, m_depth},
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
    m_depth += m_depth_step;
}

auto renderer::text(std::string const& str, glm::vec2 const& position, glm::vec4 const& color, glm::vec2 const& scale, typeface_ref_t const& tf) -> void {
    m_text_engine->text(str, {position, m_depth}, color, scale, tf);
    m_depth += m_depth_step;
}

auto renderer::text_size(std::string const& str, glm::vec2 const& scale, typeface_ref_t const& typeface) -> glm::vec2 {
    return m_text_engine->text_size(str, scale, typeface);
}

auto renderer::load_font(typeface_props const& props) -> typeface_ref_t {
    m_text_engine->load(props);
    m_text_engine->reload();
    return m_text_engine->fonts()->family(props.family)->typeface(props.style);
}
auto renderer::family(std::string const& family) -> font_family_ref_t {
    return m_text_engine->fonts()->family(family);
}
auto renderer::typeface(std::string const& family, std::string const& style) -> typeface_ref_t {
    return m_text_engine->typeface(family, style);
}
auto renderer::fonts() -> font_manager_ref_t {
    return m_text_engine->fonts();
}
auto renderer::text_engine() -> text_engine_ref_t {
    return m_text_engine;
}
} // namespace txt
