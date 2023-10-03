#include <cstdint>
#include <vector>
#include <string_view>
#include "fmt/format.h"

#include "txt/window.hpp"
#include "txt/image.hpp"
#include "txt/renderer.hpp"

static auto entry([[maybe_unused]]std::vector<std::string_view> const& args) -> void {
    auto window = txt::make_window({"Hello, Text!"});
    txt::renderer::init(window);

    auto img = txt::load_image_rgba("./nurture.jpg");
    img.fliph();
    auto texture = txt::make_texture(img.data(), img.width(), img.height(), img.channels(), {
        .internal = txt::pixel_fmt::rgba,
        .format   = txt::pixel_fmt::rgb,
    });

    txt::loop(window, [&]{
        txt::begin_frame();
        txt::viewport(0, 0, window->buffer_width(), window->buffer_height());
        txt::clear_color(0x000000);
        txt::clear();

        txt::rect({float(window->width()) / 2.0f, float(window->height()) / 2.0f}, {100.0f, 100.0f}, 0.0f);
        txt::rect({float(window->width()) / 2.0f, float(window->height()) / 2.0f}, {50.0f, 50.0f}, 0.0f, {1.0f, 0.0f, 0.0f, 1.0f});
        txt::rect({float(window->width()) / 2.0f + 50.0f, float(window->height()) / 2.0f + 50.0f}, {100.0f, 100.0f}, 0.0f, texture, {0.5f, 0.6f}, {0.5f, 0.5f});
        txt::rect({float(window->width()) / 2.0f + 100.0f, float(window->height()) / 2.0f + 100.0f}, {100.0f, 100.0f}, 0.0f);

        txt::end_frame();

        window->swap();
        window->poll();
    });
}

auto main(int argc, char const* argv[]) -> int {
    try {
        entry({argv, std::next(argv, argc)});
    } catch (std::exception const& e) {
        fmt::print(stderr, "Error at entry: {}\n", e.what());
        return 1;
    }
    return 0;
}
