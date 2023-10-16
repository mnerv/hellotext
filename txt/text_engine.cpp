#include "text_engine.hpp"

namespace txt {
text_engine::text_engine(font_manager_ref_t manager) : m_manager(manager) {
    m_manager->load({
        .filename    = "./res/fonts/Cozette/CozetteVector.ttf",
        .size        = 13,
        .family      = "Cozette",
        .style       = "Regular"
    });
}
text_engine::~text_engine() {
}

auto text_engine::text(std::string const& str, glm::vec3 const& position, glm::vec4 const& color) -> void {
    (void)str;
    (void)position;
    (void)color;
}
auto text_engine::calc_size(std::string const& str) const -> glm::vec2 {
    (void)str;
    return {};
}

auto text_engine::begin() -> void {}
auto text_engine::end() -> void {}

} // namespace txt
