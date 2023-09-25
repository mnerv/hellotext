#include "image.hpp"
#include "stb_image.h"
#include "stb_image_write.h"

namespace txt {
auto native_write_png(std::string_view const& filename, void const* data, std::size_t width, std::size_t height, std::size_t channels, std::size_t stride) -> void {
    stbi_write_png(filename.data(), std::int32_t(width), std::int32_t(height), std::int32_t(channels), data, std::int32_t(stride));
}
} // namespace txt
