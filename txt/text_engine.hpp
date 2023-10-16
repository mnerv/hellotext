#ifndef TXT_TEXT_ENGINE_HPP
#define TXT_TEXT_ENGINE_HPP

#include "utility.hpp"
#include "fonts.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace txt {
class text_engine {
public:
    text_engine(font_manager_ref_t manager);
    ~text_engine();

    auto text(std::string const& str, glm::vec3 const& position = {}, glm::vec4 const& color = glm::vec4{1.0f}) -> void;
    auto calc_size(std::string const& str) const -> glm::vec2;
    auto set_depth(float const& depth = 0.0f) -> void { m_depth = depth; }

    auto begin() -> void;
    auto end() -> void;

private:
    font_manager_ref_t m_manager;
    float m_depth{0.0f};
};

using text_engine_ref_t = ref<text_engine>;
} // namespace txt

#endif  // TXT_TEXT_ENGINE_HPP
