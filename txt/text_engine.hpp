#ifndef TXT_TEXT_ENGINE_HPP
#define TXT_TEXT_ENGINE_HPP

#include <unordered_map>

#include "utility.hpp"
#include "image.hpp"
#include "fonts.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "buffer.hpp"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace txt {
class text_engine {
public:
    text_engine(font_manager_ref_t manager);
    ~text_engine() = default;

    auto fonts() -> font_manager_ref_t { return m_manager; }
    auto typeface(std::string const& family, std::string const& style) -> typeface_ref_t;

    auto text(std::string const& str, glm::vec3 const& position = {}, glm::vec4 const& color = glm::vec4{1.0f}, typeface_ref_t const& typeface = nullptr) -> void;
    auto calc_size(std::string const& str, typeface_ref_t const& typeface = nullptr) const -> glm::vec2;

    auto set_camera(glm::mat4 const& model, glm::mat4 const& view, glm::mat4 const& projection) -> void {
        m_model      = model;
        m_view       = view;
        m_projection = projection;
    }
    auto reload() -> void;
    auto begin() -> void;
    auto end() -> void;

private:
    struct gpu {
        glm::vec4 color;
        glm::vec2 size;
        glm::vec2 uv_offset;
        glm::vec3 position;
    };

    struct batch {
        // CPU data
        image_u8_ref_t   atlas{nullptr};
        glm::ivec2       uv{0, 0};  // Starting point
        std::vector<gpu> cache{};
        std::size_t      count{0};
        std::unordered_map<std::uint32_t, glm::vec2> code_uvs{};
        std::uint32_t    size{0};

        // GPU data
        attribute_descriptor_ref_t buffer_layout{nullptr};
        vertex_buffer_ref_t        vertex_buffer{nullptr};
        texture_ref_t              texture{nullptr};

        auto push(glyph const& gh, glm::vec3 const& position, glm::vec4 const& color) -> void;
    };

    auto render_normal(batch& batch) -> void;
    auto render_subpixel(batch& batch) -> void;

private:
    auto create_batch(typeface_ref_t tf) -> void;
    auto resize_atlas(typeface_ref_t& tf, batch& batch) -> void;
    auto insert_atlas(glyph const& gh, typeface_ref_t& tf, batch& batch) -> void;

private:
    font_manager_ref_t m_manager;
    typeface_ref_t     m_typeface{nullptr};      // Default typeface
    std::unordered_map<typeface_ref_t, batch> m_batches{};  // Batches depending on rendering mode

    index_buffer_ref_t m_index_buffer{nullptr};
    shader_ref_t       m_shader_normal{nullptr};

    glm::mat4 m_model{1.0f};
    glm::mat4 m_view{1.0f};
    glm::mat4 m_projection{1.0f};
};

using text_engine_ref_t = ref<text_engine>;
} // namespace txt

#endif  // TXT_TEXT_ENGINE_HPP
