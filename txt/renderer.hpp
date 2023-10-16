#ifndef TXT_RENDERER_HPP
#define TXT_RENDERER_HPP
#include <memory>
#include <vector>
#include <map>

#include "utility.hpp"
#include "window.hpp"
#include "buffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "fonts.hpp"
#include "text_engine.hpp"

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
auto begin_frame() -> void;
auto end_frame() -> void;
auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) -> void;
auto clear_color(std::uint32_t color, float alpha = 1.0f) -> void;
auto clear(GLenum bitmask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) -> void;
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color = glm::vec4{1.0f}, glm::vec4 const& round = {}) -> void;
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, texture_ref_t texture, glm::vec2 const& uv = glm::vec2{0.0f}, glm::vec2 const& uv_size = glm::vec2{1.0f}, glm::vec4 const& color = glm::vec4{0.0f}, glm::vec4 const& round = {}) -> void;
auto text(std::string const& str, glm::vec2 const& position, glm::vec4 const& color = glm::vec4{1.0f}) -> void;
auto text_size(std::string const& str) -> glm::vec2;

// Rectangle GPU instance memory
struct gpu_rect {
    glm::vec4 color{0.0f};
    glm::vec3 position{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 rotation{0.0f};
    glm::vec2 uv_offset{0.0f};
    glm::vec2 uv_size{1.0f};
    // glm::vec4 round{0.0f};
};

class renderer {
public:
    using local_t = std::unique_ptr<renderer>;
    static auto init(window_ref_t window) -> void;

    class batch {
    private:
        shader_ref_t               m_shader;
        index_buffer_ref_t         m_index_buffer;
        vertex_buffer_ref_t        m_vertex_buffer;
        vertex_buffer_ref_t        m_instance_buffer;
        attribute_descriptor_ref_t m_layout_buffer;
    };

public:
    renderer(window_ref_t window);
    ~renderer();

    auto begin() -> void;
    auto end() -> void;
    auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width, std::uint32_t height) -> void;
    auto clear_color(std::uint32_t color, float alpha) -> void;
    auto clear(GLenum bitmask) -> void;

    auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color, glm::vec4 const& round) -> void;
    auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, texture_ref_t texture, glm::vec2 const& uv, glm::vec2 const& uv_size, glm::vec4 const& color, glm::vec4 const& round) -> void;
    auto text(std::string const& str, glm::vec2 const& position, glm::vec4 const& color) -> void;
    auto text_size(std::string const& str) -> glm::vec2;

private:
    window_ref_t m_window;
    std::vector<gpu_rect> m_color_rects{};
    std::map<texture_ref_t, std::vector<gpu_rect>> m_textures{};
    std::size_t m_color_rect_count{0};
    std::size_t m_texture_rect_count{0};
    shader_ref_t m_shader_color{nullptr};
    shader_ref_t m_shader_texture{nullptr};
    index_buffer_ref_t  m_index_buffer{nullptr};
    vertex_buffer_ref_t m_vertex_buffers{nullptr};
    attribute_descriptor_ref_t m_buffer_layout{nullptr};

    text_engine_ref_t m_text{nullptr};

private:
    float m_depth{0.0f};
    float m_depth_step{0.1f};

private:
    glm::mat4 m_model{1.0f};
    glm::mat4 m_view{};
    glm::mat4 m_projection{};
};
} // namespace txt

#endif  // TXT_RENDERER_HPP
