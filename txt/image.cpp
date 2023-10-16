#include "image.hpp"
#include "stb_image.h"
#include "stb_image_write.h"

namespace txt {
auto make_image_u8(std::uint8_t const* data, std::size_t width, std::size_t height, std::size_t channels) -> image_u8_ref_t {
    return make_ref<image_u8>(data, width, height, channels);
}

auto write_png(std::string_view const& filename, image_u8 const& img) -> void {
    auto const stride = std::int32_t(img.width() * img.channels() * sizeof(std::uint8_t));
    stbi_write_png(filename.data(), std::int32_t(img.width()), std::int32_t(img.height()), std::int32_t(img.channels()), img.data(), stride);
}

auto load_image_rgba(std::string const& filename, bool flip) -> image_u8_ref_t {
    std::int32_t width, height, channels;
    auto buffer = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    auto img = make_ref<image_u8>(buffer, width, height, channels);
    stbi_image_free(buffer);
    if (flip) img->fliph();
    return img;
}
} // namespace txt
