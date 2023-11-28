#include "image.hpp"
#include "stb_image.h"
#include "stb_image_write.h"
#include "fmt/format.h"

#include <filesystem>

namespace txt {
auto write_png(std::string_view const& filename, image_u8 const& img) -> void {
    auto const stride = std::int32_t(img.width() * img.channels() * sizeof(std::uint8_t));
    stbi_write_png(filename.data(), std::int32_t(img.width()), std::int32_t(img.height()), std::int32_t(img.channels()), img.data(), stride);
}

auto load_image(std::string const& filename, bool flip) -> image_u8 {
    if (!std::filesystem::exists(filename)) throw std::runtime_error(fmt::format("Image file '{}' does not exist!", filename));
    std::int32_t width, height, channels;
    auto buffer = stbi_load(filename.c_str(), &width, &height, &channels, 0);
    auto img = image_u8(buffer, std::size_t(width), std::size_t(height), std::size_t(channels));
    stbi_image_free(buffer);
    if (flip) img.fliph();
    return img;
}
} // namespace txt
