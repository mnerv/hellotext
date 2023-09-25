#include "descriptor.hpp"

#include <vector>
#ifdef __EMSCRIPTEN__
#include "GL/gl.h"
#else
#include "glad/glad.h"
#endif

namespace txt {
using layout_descriptor_ref_t = std::shared_ptr<layout_descriptor>;
auto make_layout_descriptor(std::initializer_list<attribute_description> const& descriptions) -> layout_descriptor_ref_t {
    return std::make_shared<layout_descriptor>(descriptions);
}

layout_descriptor::layout_descriptor() {}
layout_descriptor::layout_descriptor(std::initializer_list<attribute_description> const& descriptions) {
    init(descriptions);
}
layout_descriptor::~layout_descriptor() {
    clean_up();
}

auto layout_descriptor::init(std::initializer_list<attribute_description> const& descriptions) -> void {
    if (!m_layout.empty()) clean_up();
    m_layout = descriptions;
    glGenVertexArrays(1, &m_id);
    bind();
}
auto layout_descriptor::bind() -> void {
    glBindVertexArray(m_id);
}
auto layout_descriptor::unbind() -> void {
    glBindVertexArray(0);
}

auto layout_descriptor::clean_up() -> void {
    glDeleteVertexArrays(1, &m_id);
    m_id = 0;
}
} // namespace txt
