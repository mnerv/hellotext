#ifndef TXT_RENDER_HPP
#define TXT_RENDER_HPP
#include <memory>
#include <vector>

#include "window.hpp"
#include "buffer.hpp"
#include "descriptor.hpp"

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
struct rectangle {
    glm::vec4 color;
    glm::mat4 model;
    glm::mat3 uv{1.0f};
    glm::vec4 round{0.0f};
};

auto begin_frame() -> void;
auto end_frame() -> void;
auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) -> void;
auto clear_color(std::uint32_t color, float alpha = 1.0f) -> void;
auto clear(GLenum bitmask = GL_COLOR_BUFFER_BIT) -> void;
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
    window::ref_t          m_window;
    std::vector<rectangle> m_gpu_rects{};
    std::size_t            m_gpu_rects_count{0};
    buffer m_rect_ib{};
    buffer m_rect_vb{};
    buffer m_rect_ivb{};
    layout_descriptor m_rect_descriptor{};
};
} // namespace txt

#endif  // TXT_RENDER_HPP
