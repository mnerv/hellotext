#ifndef TXT_RENDERER_HPP
#define TXT_RENDERER_HPP
#include <memory>
#include <vector>
#include <utility>
#include <map>

#include "types.hpp"
#include "utils.hpp"
#include "window.hpp"
#include "text/fonts.hpp"
#include "graphics/buffer.hpp"
#include "graphics/shader.hpp"
#include "graphics/texture.hpp"
#include "text/engine.hpp"

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

#ifdef __EMSCRIPTEN__
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#define EGL_EGLEXT_PROTOTYPES 1
#include "GL/gl.h"
#else
#include "glad/glad.h"
#endif

namespace txt {
constexpr GLenum GL_DEFAULT_CLEAR = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

/**
 * Convert HSB value to RGB.
 * @param hue        - Hue value with range [0, 360]
 * @param saturation - Saturation value with range [0, 1]
 * @param brightness - Brightness value with range [0, 1]
 * @return RGB with range [0, 1]
*/
auto hsb2rgb(f32 hue, f32 saturation, f32 brightness) -> glm::vec3;

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
            auto const texture_hash = std::hash<txt::texture_ref_t>{}(mat.texture);
            return shader_hash ^ (texture_hash << 1);
        }
    };
};

class renderer {
public:
    using local_t = std::unique_ptr<renderer>;

public:
    renderer(window_ref_t window);
    ~renderer();

    auto load_font(font_load_params const& params) -> void;

    auto begin() -> void;
    auto end() -> void;

    auto viewport(std::int32_t x, std::int32_t y,
                  std::uint32_t width, std::uint32_t height) -> void;

    auto clear_color(std::uint32_t color, float alpha = 1.0f) -> void;

    auto clear(GLenum bitmask = GL_DEFAULT_CLEAR) -> void;

    auto rect(glm::vec2 const& position, glm::vec2 const& size,
              float const& rotation, glm::vec4 const& color,
              glm::vec4 const& round) -> void;

    auto rect(glm::vec2 const& position, glm::vec2 const& size,
              float const& rotation, shader_ref_t shader,
              texture_ref_t texture, glm::vec2 const& uv,
              glm::vec2 const& uv_size, float const& z_offset = 0.0f) -> void;

    auto text(std::string const& str, glm::vec2 const& position,
              glm::vec4 const& color = {1.0f, 1.0f, 1.0f, 1.0f},
              glm::vec2 const& scale = {1.0f, 1.0f}) -> void;

    auto text_size(std::string const& str,
                   glm::vec2 const& scale) -> glm::vec2;

    auto zdepth() const -> float;

private:
    using rect_instances_t   = std::vector<rect_instance>;
    using rect_texture_map_t = std::unordered_map<shader_texture_pair,
                                                  rect_instances_t,
                                                  shader_texture_pair::hash>;

private:
    window_ref_t      m_window;
    text_engine_ref_t m_text_engine{nullptr};
    shader_ref_t      m_rect_default_shader{nullptr};
    shader_ref_t      m_rect_texture_shader{nullptr};
    // Base rectangle batch
    index_buffer_ref_t  m_rect_index_buffer{nullptr};
    vertex_buffer_ref_t m_rect_vertex_buffer{nullptr};
    attribute_descriptor_ref_t m_rect_descriptor{nullptr};

    std::size_t m_color_rect_size{0};
    rect_instances_t   m_color_rects{};
    rect_texture_map_t m_shader_texture_rects{};

private:
    f32 m_depth{0.0f};
    f32 m_depth_step{0.1f};

private:
    glm::mat4 m_model{1.0f};
    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};
};

using renderer_local_t = local<renderer>;
auto make_renderer(window_ref_t window) -> renderer_local_t;
}  // namespace txt

#endif  // TXT_RENDERER_HPP
