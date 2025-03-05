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

#include "fonts.hpp"

namespace txt {
class text_engine {
public:
    text_engine(window_ref_t window);
    ~text_engine() = default;

    auto load(font_load_params const& params) -> void;
    auto draw(std::string const& str, glm::vec2 const& position, glm::vec4 const& color, glm::vec2 const& scale) -> void;
    auto text_size(std::string const& str, glm::vec2 const& scale = glm::vec2{1.0}) -> glm::vec2;
    auto text_size(std::string const& str, glm::vec2 const& scale, font const& font) -> glm::vec2;
    auto set_default(font& font) -> void;

private:
    friend class renderer;
    auto begin() -> void;
    auto end() -> void;

private:
    window_ref_t m_window;
    font_manager m_manager{};
    font const*  m_current{};
};

using text_engine_ref_t = ref<text_engine>;
} // namespace txt

#endif  // TXT_TEXT_ENGINE_HPP
