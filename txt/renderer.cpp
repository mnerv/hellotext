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

auto renderer::init(window::ref_t window) -> void {
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

auto renderer::begin() -> void {
    m_gpu_rects_count = 0;
}

auto renderer::end() -> void {
    auto const instance_bytes = m_gpu_rects_count * sizeof(gpu_rect);
    m_rects_vb->bind();
    m_rects_vb->resize(instance_bytes);
    m_rects_vb->sub(m_gpu_rects.data(), instance_bytes, 0);
    m_rects_vb->unbind();

    m_view = glm::lookAt(glm::vec3{0.0, 0.0, 1.0}, glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0});
    m_projection = glm::ortho(0.0f, float(m_window->width()), 0.0f, float(m_window->height()));

    m_shader->bind();
    m_shader->upload_mat4("u_model", m_model);
    m_shader->upload_mat4("u_view", m_view);
    m_shader->upload_mat4("u_projection", m_projection);

    m_rects->bind();
    m_rect_ib->bind();
    glDrawElementsInstanced(GL_TRIANGLES, GLsizei(m_rect_ib->size()), gl_type(m_rect_ib->type()), nullptr, GLsizei(m_gpu_rects_count));
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
        color,
        {position.x, position.y, 0.0f},
        {size, 1.0f},
        {0.0f, 0.0f, rotation},
        {0.0f, 0.0f},
        {1.0f, 1.0f},
    };

    if ( m_gpu_rects_count < m_gpu_rects.size()) {
        m_gpu_rects[m_gpu_rects_count] = rect;
    } else {
        m_gpu_rects.push_back(rect);
    }
    ++m_gpu_rects_count;
}

renderer::renderer(window::ref_t window) : m_window(window) {
    m_shader = make_shader(
        read_text("./shaders/opengl/texture.vert"),
        read_text("./shaders/opengl/color.frag")
    );

    m_rect_ib = make_index_buffer(QUAD_INDICES_CW, sizeof(QUAD_INDICES_CW), len(QUAD_INDICES_CW), type::u32, usage::static_draw);
    m_rects = make_attribute_descriptor();
    m_rects->add(make_vertex_buffer(QUAD_VERTICES, sizeof(QUAD_VERTICES), type::f32, usage::static_draw), {
        {type::vec3},
        {type::vec2}
    });
    m_rects_vb = make_vertex_buffer(nullptr, sizeof(gpu_rect), type::f32, usage::dynamic_draw);
    m_rects->add(m_rects_vb, {
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
