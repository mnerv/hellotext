#include "texture.hpp"

#ifndef __EMSCRIPTEN__
#include "glad/glad.h"
#else
#include "GL/gl.h"
#endif

namespace txt {
auto make_texture(void const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels, texture_props const& props) -> texture_ref_t {
    return make_ref<texture>(data, width, height, channels, props);
}

constexpr auto gl_texture_format(pixel_fmt value) -> GLenum {
    switch (value) {
        case pixel_fmt::rgb:  return GL_RGB;
        case pixel_fmt::rgba: return GL_RGBA;
        default: return GL_RGBA;
    }
}

constexpr auto gl_texture_wrap(tex_wrap value) -> GLenum {
    switch (value) {
        case tex_wrap::repeat: return GL_REPEAT;
        case tex_wrap::mirrored_repeat: return GL_MIRRORED_REPEAT;
        case tex_wrap::clamp_to_edge:   return GL_CLAMP_TO_EDGE;
        case tex_wrap::clamp_to_border: return GL_CLAMP_TO_BORDER;
        case tex_wrap::mirror_clamp_to_edge: return GL_MIRROR_CLAMP_TO_EDGE;
        default: return GL_REPEAT;
    }
}

constexpr auto gl_texture_filter(tex_filter value) -> GLenum {
    switch (value) {
        case tex_filter::nearest: return GL_NEAREST;
        case tex_filter::linear:  return GL_LINEAR;
        case tex_filter::nearest_mipmap_nearest: return GL_NEAREST_MIPMAP_NEAREST;
        case tex_filter::linear_mipmap_nearest:  return GL_LINEAR_MIPMAP_NEAREST;
        case tex_filter::nearest_mipmap_linear:  return GL_NEAREST_MIPMAP_LINEAR;
        case tex_filter::linear_mipmap_linear:   return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_NEAREST;
    }
}

texture::texture(void const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels, texture_props const& props)
    : m_id(0)
    , m_width(width)
    , m_height(height)
    , m_channels(channels) {
    glGenTextures(1, &m_id);
    set(data, width, height, channels, props);
}
texture::~texture() {
    glDeleteTextures(1, &m_id);
}

auto texture::set(void const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels, texture_props const& props) -> void {
    m_width    = width;
    m_height   = height;
    m_channels = channels;

    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, gl_texture_format(props.internal), GLsizei(m_width), GLsizei(m_height), 0, gl_texture_format(props.format), GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_texture_wrap(props.wrap_s));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_texture_wrap(props.wrap_t));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_texture_filter(props.min_filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_texture_filter(props.mag_filter));
    if (props.mipmap) glGenerateMipmap(GL_TEXTURE_2D);
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

