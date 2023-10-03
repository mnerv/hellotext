#include "image.hpp"
#include "stb_image.h"
#include "stb_image_write.h"

namespace txt {
auto write_png(std::string_view const& filename, image_u8 const& img) -> void {
    auto const stride = std::int32_t(img.width() * img.channels() * sizeof(std::uint8_t));
    stbi_write_png(filename.data(), std::int32_t(img.width()), std::int32_t(img.height()), std::int32_t(img.channels()), img.data(), stride);
}

auto load_image_rgba(std::string const& filename) -> image_u8 {
    std::int32_t width, height, channels;
    auto buffer = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    image_u8 img(buffer, width, height, channels);
    stbi_image_free(buffer);
    return img;
}
} // namespace txt
