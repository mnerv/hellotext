#ifndef TXT_RENDER_HPP
#define TXT_RENDER_HPP
#include <memory>
#include <vector>

#include "utility.hpp"
#include "window.hpp"
#include "buffer.hpp"
#include "shader.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#ifndef __EMSCRIPTEN__
#include "glad/glad.h"
#else
#include "GL/gl.h"
#endif

namespace txt {
// Rectangle GPU instance memory
struct gpu_rect {
    glm::vec4 color{0.0f};
    glm::vec3 position{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 rotation{0.0f};
    glm::vec2 uv_offset{0.0f};
    glm::vec2 uv_size{1.0f};
    // glm::vec4 round;
};

auto begin_frame() -> void;
auto end_frame() -> void;
auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) -> void;
auto clear_color(std::uint32_t color, float alpha = 1.0f) -> void;
auto clear(GLenum bitmask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) -> void;
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color = glm::vec4{1.0f}) -> void;

class renderer {
public:
    using local_t = std::unique_ptr<renderer>;
    static auto init(window::ref_t window) -> void;

public:
    renderer(window::ref_t window);
    ~renderer();

    auto begin() -> void;
    auto end() -> void;
    auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) -> void;
    auto clear_color(std::uint32_t color, float alpha) -> void;
    auto clear(GLenum bitmask) -> void;

    auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color) -> void;

private:
    window::ref_t         m_window;
    std::vector<gpu_rect> m_gpu_rects{};
    std::size_t           m_gpu_rects_count{0};
    shader_ref_t          m_shader{nullptr};
    index_buffer_ref_t    m_rect_ib;
    vertex_buffer_ref_t   m_rects_vb;
    attribute_descriptor_ref_t m_rects;

private:
    glm::mat4 m_model{1.0f};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
};
} // namespace txt

#endif  // TXT_RENDER_HPP
