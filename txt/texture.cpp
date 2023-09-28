#include "texture.hpp"

#ifndef __EMSCRIPTEN__
#include "glad/glad.h"
#else
#include "GL/gl.h"
#endif

namespace txt {
texture::texture(void const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels, texture_props const& props)
    : m_id(0)
    , m_width(width)
    , m_height(height)
    , m_channels(channels) {
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    (void)data;
    (void)props;
}

auto texture::bind(std::size_t const& slot) const -> void {
    glActiveTexture(GL_TEXTURE0 + std::uint32_t(slot));
    glBindTexture(GL_TEXTURE_2D, m_id);
}
auto texture::unbind(std::size_t const& slot) const -> void {
    glActiveTexture(GL_TEXTURE0 + std::uint32_t(slot));
    glBindTexture(GL_TEXTURE_2D, 0);
}
}

