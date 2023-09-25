#include "renderer.hpp"
#include <stdexcept>
#include "fmt/format.h"

namespace txt {
static renderer::local_t s_instance = nullptr;
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
    // TODO: Implement end
    if (m_gpu_rects.size() - m_gpu_rects_count > 256) {
        m_gpu_rects.resize(m_gpu_rects_count);
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
    (void)position;
    (void)size;
    (void)rotation;
    (void)color;
    // ++m_gpu_rects_count;
}

renderer::renderer(window::ref_t window) : m_window(window) {
}
renderer::~renderer() {
}
} // namespace txt
