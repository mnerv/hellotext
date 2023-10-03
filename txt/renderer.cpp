#include "renderer.hpp"
#include <stdexcept>
#include "fmt/format.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace txt {
static renderer::local_t s_instance = nullptr;
[[maybe_unused]]static constexpr float QUAD_VERTICES[]{
//     x,    y,    z,     u,   v
    -0.5, -0.5,  0.0,   0.0, 0.0,
    -0.5,  0.5,  0.0,   0.0, 1.0,
     0.5,  0.5,  0.0,   1.0, 1.0,
     0.5, -0.5,  0.0,   1.0, 0.0,
};
[[maybe_unused]]static constexpr std::initializer_list<attribute_description> QUAD_LAYOUT{
    {txt::type::vec3, 0},  // position
    {txt::type::vec2, 0},  // uv
};
[[maybe_unused]]static constexpr std::uint32_t QUAD_INDICES_CW[]{
    0, 1, 2,
    0, 2, 3,
};

auto renderer::init(window_ref_t window) -> void {
    if (s_instance != nullptr) throw std::runtime_error("txt::render has already been initialised!");
    s_instance = std::make_unique<renderer>(window);
}

auto begin_frame() -> void {
    s_instance->begin();
}
auto end_frame() -> void {
    s_instance->end();
}
auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) -> void {
    s_instance->viewport(x, y, width, height);
}
auto clear_color(std::uint32_t color, float alpha) -> void {
    s_instance->clear_color(color, alpha);
}
auto clear(GLenum bitmask) -> void {
    s_instance->clear(bitmask);
}
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color) -> void {
    s_instance->rect(position, size, rotation, color);
}
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, texture_ref_t texture, glm::vec2 const& uv, glm::vec2 const& uv_size, glm::vec4 const& color) -> void {
    s_instance->rect(position, size, rotation, texture, uv, uv_size, color);
}

auto renderer::begin() -> void {
    m_depth = 0.0f;
    m_textures.clear();
    m_color_rect_count = 0;
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

auto renderer::end() -> void {
    m_view = glm::lookAt(glm::vec3{0.0, 0.0, 1023.0}, glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0});
    m_projection = glm::ortho(0.0f, float(m_window->width()), 0.0f, float(m_window->height()), 0.1f, 1024.0f);

    if (m_color_rect_count > 0) {
        auto const bytes = m_color_rect_count * sizeof(gpu_rect);
        m_vertex_buffers->bind();
        m_vertex_buffers->resize(bytes);
        m_vertex_buffers->sub(m_color_rects.data(), bytes, 0);
        m_vertex_buffers->unbind();

        m_shader_color->bind();
        m_shader_color->upload_mat4("u_model", m_model);
        m_shader_color->upload_mat4("u_view", m_view);
        m_shader_color->upload_mat4("u_projection", m_projection);

        m_buffer_layout->bind();
        m_index_buffer->bind();
        glDrawElementsInstanced(GL_TRIANGLES, GLsizei(m_index_buffer->size()), gl_type(m_index_buffer->type()), nullptr, GLsizei(m_color_rect_count));
    }

    auto is_first = true;
    for (auto const& [texture, rects] : m_textures) {
        auto const bytes = rects.size() * sizeof(gpu_rect);
        if (bytes == 0) continue;

        m_vertex_buffers->bind();
        m_vertex_buffers->resize(bytes);
        m_vertex_buffers->sub(rects.data(), bytes, 0);
        m_vertex_buffers->unbind();

        if (is_first) {
            is_first = false;
            m_shader_texture->bind();
            m_shader_texture->upload_num("u_texture", 0.0f);
            m_shader_texture->upload_mat4("u_model", m_model);
            m_shader_texture->upload_mat4("u_view", m_view);
            m_shader_texture->upload_mat4("u_projection", m_projection);
        }

        texture->bind();
        m_buffer_layout->bind();
        m_index_buffer->bind();
        glDrawElementsInstanced(GL_TRIANGLES, GLsizei(m_index_buffer->size()), gl_type(m_index_buffer->type()), nullptr, GLsizei(rects.size()));
    }
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

auto renderer::rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color) -> void {
    gpu_rect rect{
        .color     = color,
        .position  = {position.x, position.y, m_depth},
        .scale     = {size, 1.0f},
        .rotation  = {0.0f, 0.0f, rotation},
        .uv_offset = {0.0f, 0.0f},
        .uv_size   = {1.0f, 1.0f},
    };
    if (m_color_rect_count < m_color_rects.size()) {
        m_color_rects[m_color_rect_count] = rect;
    } else {
        m_color_rects.push_back(rect);
    }
    ++m_color_rect_count;
    m_depth += m_depth_step;
}
auto renderer::rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, texture_ref_t texture, glm::vec2 const& uv, glm::vec2 const& uv_size, glm::vec4 const& color) -> void {
    gpu_rect rect{
        .color     = color,
        .position  = {position.x, position.y, m_depth},
        .scale     = {size, 1.0f},
        .rotation  = {0.0f, 0.0f, rotation},
        .uv_offset = uv,
        .uv_size   = uv_size,
    };
    m_textures[texture].push_back(rect);
    m_depth += m_depth_step;
}

renderer::renderer(window_ref_t window) : m_window(window) {
    m_shader_color = make_shader(
        read_text("./shaders/opengl/base.vert"),
        read_text("./shaders/opengl/color.frag")
    );
    m_shader_texture = make_shader(
        read_text("./shaders/opengl/base.vert"),
        read_text("./shaders/opengl/texture.frag")
    );
    m_index_buffer = make_index_buffer(QUAD_INDICES_CW, sizeof(QUAD_INDICES_CW), len(QUAD_INDICES_CW), type::u32, usage::static_draw);
    m_buffer_layout = make_attribute_descriptor();
    m_buffer_layout->add(make_vertex_buffer(QUAD_VERTICES, sizeof(QUAD_VERTICES), type::f32, usage::static_draw), {
        {type::vec3},
        {type::vec2}
    });
    m_vertex_buffers = make_vertex_buffer(nullptr, sizeof(gpu_rect), type::f32, usage::dynamic_draw);
    m_buffer_layout->add(m_vertex_buffers, {
        {type::vec4, false, 1},
        {type::vec3, false, 1},
        {type::vec3, false, 1},
        {type::vec3, false, 1},
        {type::vec2, false, 1},
        {type::vec2, false, 1},
    });
}
renderer::~renderer() {
}
} // namespace txt
