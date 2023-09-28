#ifndef TXT_IMAGE_HPP
#define TXT_IMAGE_HPP
#include <cstdint>
#include <cstddef>
#include <array>
#include <cstring>
#include <string_view>

namespace txt {
template <typename T, std::size_t C = 3>
class image {
public:
    using pointer_type = T*;
    using pixel_type   = std::array<T, C>;

public:
    image(std::size_t width, std::size_t height)
        : m_buffer(nullptr)
        , m_width(width)
        , m_height(height)
        , m_channels(C)
        , m_size(m_width * m_height * m_channels) {
        m_buffer = new T[m_size];
        std::memset(m_buffer, 0, m_size * sizeof(T));
    }
    ~image() {
        delete[] m_buffer;
    }

    auto resize(std::size_t width, std::size_t height) -> void {
        delete[] m_buffer;
        m_width  = width;
        m_height = height;
        m_size   = m_width * m_height * m_channels;
        m_buffer = new T[m_size];
    }
    auto width() const noexcept -> std::size_t { return m_width; }
    auto height() const noexcept -> std::size_t { return m_height; }
    auto channels() const noexcept -> std::size_t { return m_channels; }
    auto size() const noexcept -> std::size_t { return m_size; }
    auto bytes() const noexcept -> std::size_t { return m_size * sizeof(T); }
    auto data() const noexcept -> T const* { return m_buffer; }
    auto pixel(std::size_t x, std::size_t y) const noexcept -> pixel_type {
        pixel_type color{};
        if (!is_valid_range(x, y)) return color;
        auto const index = pixel_index(x, y);
        for (std::size_t i = 0; i < m_channels; ++i)
            color[i] = m_buffer[index + i];
        return color;
    }
    auto set(std::size_t x, std::size_t y, pixel_type const& color) noexcept -> void {
        if (!is_valid_range(x, y)) return;
        auto const index = pixel_index(x, y);
        for (std::size_t i = 0; i < m_channels; ++i)
            m_buffer[index + i] = color[i];
    }

    auto fliph() noexcept -> void {
        for (std::size_t i = 0; i < m_height / 2; i++) {
            for (std::size_t j = 0; j < m_width; j++) {
                auto const a = pixel(j, i);
                auto const b = pixel(j, m_height - i - 1);
                set(j, i, b);
                set(j, m_height - i - 1, a);
            }
        }
    }
    auto flipv() noexcept -> void {
        for (std::size_t i = 0; i < m_height; i++) {
            for (std::size_t j = 0; j < m_width / 2; j++) {
                auto const a = pixel(j, i);
                auto const b = pixel(j - m_width - 1, i);
                set(j, i, b);
                set(j - m_width - 1, i, a);
            }
        }
    }

private:
    auto is_valid_range(std::size_t x, std::size_t y) const noexcept -> bool {
        return x < m_width && y < m_height;
    }
    auto pixel_index(std::size_t x, std::size_t y) const noexcept -> std::size_t {
        return (y * m_channels * m_width) + (x * m_channels);
    }

private:
    pointer_type m_buffer;
    std::size_t  m_width;
    std::size_t  m_height;
    std::size_t  m_channels;
    std::size_t  m_size;
};

auto native_write_png(std::string_view const& filename, void const* data, std::size_t width, std::size_t height, std::size_t channels, std::size_t stride) -> void;

using image_rgb  = image<std::uint8_t>;
using image_rgba = image<std::uint8_t, 4>;

template <typename T, std::size_t C>
auto write_png(std::string_view const& filename, image<T, C> const& img) -> void {
    native_write_png(filename, img.data(), img.width(), img.height(), img.channels(), img.width() * img.channels() * sizeof(T));
}
} // namespace txt

#endif  // TXT_IMAGE_HPP
