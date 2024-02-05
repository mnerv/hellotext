#include <cstdint>
#include <vector>
#include <string_view>
#include <random>
#include <tuple>

#include "fmt/format.h"

#include "txt/window.hpp"
#include "txt/image.hpp"
#include "txt/renderer.hpp"

static auto entry([[maybe_unused]]std::vector<std::string_view> const& args) -> void {
    auto window = txt::make_window({"Hello, Text!"});
    txt::renderer::init(window);
    window->setup();

    window->add_event_listener([&](txt::key_down_event const& e) {
        fmt::print("{}\n", e.str());
    });
    window->add_event_listener([&](txt::key_up_event const& e) {
        if (e.keycode() == txt::keycode::Q) window->close();
        if (e.keycode() == txt::keycode::F) window->fullscreen();
    });

    txt::loop(window, [&] (double){
        // txt::begin_frame();
        // txt::viewport(0, 0, window->buffer_width(), window->buffer_height());
        // txt::clear_color(0x000000);
        // txt::clear();
        //
        // txt::rect({125.0f, 125.0f}, {100.0f, 50.0f});
        // txt::rect({125.0f, 125.0f}, {2.0f, 2.0f}, 0.0f, {1.0f, 0.0f, 0.0f, 1.0f});
        //
        // txt::end_frame();

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
