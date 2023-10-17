#ifndef TXT_TEXT_ENGINE_HPP
#define TXT_TEXT_ENGINE_HPP

#include <unordered_map>

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
    auto insert_bitmap(txt::glyph const& glyph) -> void;

private:
    typeface_ref_t   m_typeface;
    std::vector<gpu> m_data;
};

class text_engine {
public:
    text_engine(window_ref_t window, font_manager_ref_t manager);
    ~text_engine() = default;

    auto fonts() -> font_manager_ref_t { return m_manager; }
    auto typeface(std::string const& family, std::string const& style) -> typeface_ref_t;

    auto text(std::string const& str, glm::vec3 const& position = {}, glm::vec4 const& color = glm::vec4{1.0f}, typeface_ref_t const& typeface = nullptr, glm::vec3 const& scale = glm::vec3{1.0f}) -> void;
    auto text_size(std::string const& str, typeface_ref_t const& typeface = nullptr, glm::vec3 const& scale = glm::vec3{1.0f}) const -> glm::vec2;

    auto set_camera(glm::mat4 const& model, glm::mat4 const& view, glm::mat4 const& projection) -> void {
        m_model      = model;
        m_view       = view;
        m_projection = projection;
    }
    auto reload() -> void;
    auto begin() -> void;
    auto end() -> void;

private:
    window_ref_t       m_window;
    font_manager_ref_t m_manager;
    typeface_ref_t     m_typeface{nullptr};      // Default typeface

    index_buffer_ref_t  m_index_buffer{nullptr};
    vertex_buffer_ref_t m_instance_buffer{nullptr};
    attribute_descriptor_ref_t m_descriptor{nullptr};

    shader_ref_t       m_shader_normal{nullptr};

    glm::mat4 m_model{1.0f};
    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};
};

using text_engine_ref_t = ref<text_engine>;
} // namespace txt

#endif  // TXT_TEXT_ENGINE_HPP
