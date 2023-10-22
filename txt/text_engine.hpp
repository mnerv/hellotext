#ifndef TXT_TEXT_ENGINE_HPP
#define TXT_TEXT_ENGINE_HPP

#include <map>

#include "utility.hpp"
#include "window.hpp"
#include "image.hpp"
#include "fonts.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "buffer.hpp"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace txt {
class text_batch {
public:
    struct gpu {
        glm::vec4 color;
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec2 uv_offset;
        glm::vec2 uv_size;
    };

public:
    text_batch(typeface_ref_t typeface);
    ~text_batch() = default;

    auto size() const -> std::size_t { return m_size; }
    auto chars() const -> std::vector<gpu> const& { return m_data; }
    auto texture() const -> texture_ref_t const& { return m_texture; }
    auto bitmap() const -> image_u8_ref_t const& { return m_atlas; }
    auto max_delta_origin_ymin() const -> std::int32_t { return m_max_delta_origin_ymin; }
    auto max_bearing_left() const -> std::int32_t { return m_max_bearing_left; }
    auto max_bearing_top() const -> std::int32_t { return m_max_bearing_top; }
    auto generate_atlas() -> void;
    auto reset() -> void;
    auto push(glyph const& code, glm::vec3 const& position, glm::vec4 const& color = glm::vec4{1.0f}, glm::vec2 const& scale = glm::vec2{1.0f}) -> void;
    auto insert_bitmap(txt::glyph const& glyph) -> void;

private:
    auto resize_atlas() -> void;

private:
    typeface_ref_t   m_typeface;
    bool             m_is_typeface_valid{false};
    std::vector<gpu> m_data{};
    std::size_t      m_size{0};
    image_u8_ref_t   m_atlas{nullptr};
    std::map<std::uint32_t, glm::vec2> m_uv_map{};
    glm::ivec2    m_current_uv{0, 0};
    texture_ref_t m_texture{nullptr};
    std::int32_t  m_max_delta_origin_ymin{0};
    std::int32_t  m_max_bearing_top{0};
    std::int32_t  m_max_bearing_left{0};
};

class text_engine {
public:
    text_engine(window_ref_t window, font_manager_ref_t manager);
    ~text_engine() = default;

    auto fonts() -> font_manager_ref_t { return m_manager; }
    auto typeface(std::string const& family, std::string const& style) -> typeface_ref_t;

    auto text(std::string const& str, glm::vec3 const& position = {}, glm::vec4 const& color = glm::vec4{1.0f}, glm::vec2 const& scale = glm::vec2{1.0f}, typeface_ref_t const& typeface = nullptr) -> void;
    auto text_size(std::string const& str, glm::vec2 const& scale = glm::vec2{1.0f}, typeface_ref_t const& typeface = nullptr) -> glm::vec2;

    auto load(typeface_props const props) -> void;
    auto set_camera(glm::mat4 const& view, glm::mat4 const& projection) -> void {
        m_view       = view;
        m_projection = projection;
    }
    auto reload() -> void;
    auto begin() -> void;
    auto end() -> void;

private:
    auto render_normal(text_batch const& batch) -> void;
    auto render_subpixel(text_batch const& batch) -> void;

private:
    window_ref_t       m_window;
    font_manager_ref_t m_manager;
    typeface_ref_t     m_typeface{nullptr};      // Default typeface

    index_buffer_ref_t  m_index_buffer{nullptr};
    vertex_buffer_ref_t m_instance_buffer{nullptr};
    attribute_descriptor_ref_t m_descriptor{nullptr};

    shader_ref_t m_shader_normal{nullptr};
    std::map<typeface_ref_t, text_batch> m_batches{};

    glm::mat4 m_model{1.0f};
    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};
};

using text_engine_ref_t = ref<text_engine>;
} // namespace txt

#endif  // TXT_TEXT_ENGINE_HPP
