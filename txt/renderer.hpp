#ifndef TXT_RENDERER_HPP
#define TXT_RENDERER_HPP
#include <memory>
#include <vector>
#include <utility>
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
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation = 0.0f, glm::vec4 const& color = glm::vec4{1.0f}, glm::vec4 const& round = {}) -> void;
auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, texture_ref_t texture, glm::vec2 const& uv = {0.0f, 0.0f}, glm::vec2 const& uv_size = {1.0f, 1.0f}, glm::vec4 const& round = {0.0f, 0.0f, 0.0f, 0.0f}) -> void;
auto text(std::string const& str, glm::vec2 const& position, glm::vec4 const& color = glm::vec4{1.0f}, glm::vec2 const& scale = glm::vec2{1.0f}) -> void;
auto text_size(std::string const& str, glm::vec2 const& scale = glm::vec2{1.0f}) -> glm::vec2;

/**
 * Convert HSB value to RGB.
 * @param hue        - Hue value with range [0, 360]
 * @param saturation - Saturation value with range [0, 1]
 * @param brightness - Brightness value with range [0, 1]
 * @return RGB with range [0, 1]
*/
auto hsb2rgb(float hue, float saturation, float brightness) -> glm::vec3;

struct rect_instance {
    glm::vec4 color{0.0f};
    glm::vec3 position{0.0f};
    glm::vec3 scale{1.0f};
    glm::vec3 rotation{0.0f};
    glm::vec2 uv_offset{0.0f};
    glm::vec2 uv_size{1.0f};
};

struct shader_texture_pair {
    shader_ref_t shader;
    texture_ref_t texture;
    mutable std::size_t size;
    // TODO: Check for stale pair to remove them

    shader_texture_pair(shader_ref_t shader_ref, texture_ref_t texture_ref)
        : shader(shader_ref), texture(texture_ref), size(0) {}

    auto operator==(shader_texture_pair const& other) const -> bool {
        return shader == other.shader && texture == other.texture;
    }

    struct hash {
        auto operator()(shader_texture_pair const& mat) const -> std::size_t {
            auto const shader_hash = std::hash<txt::shader_ref_t>{}(mat.shader);
            auto const texture_hash =
                std::hash<txt::texture_ref_t>{}(mat.texture);
            return shader_hash ^ (texture_hash << 1);
        }
    };
};

class renderer {
public:
    using local_t = std::unique_ptr<renderer>;
    static auto init(window_ref_t window) -> void;
    static auto instance() -> local_t&;

public:
    renderer(window_ref_t window);
    ~renderer();

    auto begin() -> void;
    auto end() -> void;
    static auto viewport(std::int32_t x, std::int32_t y, std::uint32_t width,
                  std::uint32_t height) -> void;
    static auto clear_color(std::uint32_t color, float alpha) -> void;
    static auto clear(GLenum bitmask) -> void;

    auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, glm::vec4 const& color, glm::vec4 const& round) -> void;
    auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, texture_ref_t texture, glm::vec2 const& uv, glm::vec2 const& uv_size, glm::vec4 const& round) -> void;
    auto rect(glm::vec2 const& position, glm::vec2 const& size, float const& rotation, shader_ref_t shader, texture_ref_t texture, glm::vec2 const& uv, glm::vec2 const& uv_size, [[maybe_unused]] glm::vec4 const& round) -> void;
    auto text(std::string const& str, glm::vec2 const& position, glm::vec4 const& color, glm::vec2 const& scale) -> void;
    auto text_size(std::string const& str, glm::vec2 const& scale) -> glm::vec2;

   private:
    window_ref_t m_window;
    shader_ref_t m_rect_default_shader;
    shader_ref_t m_rect_texture_shader;
    // Base rectangle batch
    index_buffer_ref_t m_rect_index_buffer;
    vertex_buffer_ref_t m_rect_vertex_buffer;
    attribute_descriptor_ref_t m_rect_descriptor;

    std::size_t m_color_rect_size{0};
    std::vector<rect_instance> m_color_rects{};
    std::unordered_map<shader_texture_pair, std::vector<rect_instance>, shader_texture_pair::hash> m_shader_texture_rects{};

private:
    float m_depth{0.0f};
    float m_depth_step{0.1f};

private:
    glm::mat4 m_model{1.0f};
    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};
};
}  // namespace txt

#endif  // TXT_RENDERER_HPP
