#include <cstdint>
#include <vector>
#include <string_view>
#include "fmt/format.h"

#include "txt/window.hpp"
#include "txt/image.hpp"
#include "txt/renderer.hpp"

static auto entry([[maybe_unused]]std::vector<std::string_view> const& args) -> void {
    auto window = txt::window::make({"Hello, Text!"});
    txt::renderer::init(window);

    txt::loop(window, [&]{
        txt::begin_frame();
        txt::viewport(0, 0, window->buffer_width(), window->buffer_height());
        txt::clear_color(0x000000);
        txt::clear();

        auto const max = 100.0f;
        for (float i = 0.0f; i < max; ++i) {
            auto const radius = 500.0f;
            auto const t = window->time();
            auto const x = radius * std::cos((i + t) * (2.0f * 3.14f) / max) + float(window->width()) / 2.0f;
            auto const y = radius * std::sin((i + t) * (2.0f * 3.14f) / max) + float(window->height()) / 2.0f;
            txt::rect({x, y}, {10.0f, 10.0f}, 0.0f, {1.0f, 0.0f, 0.0f, 1.0f});
        }

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
