#include <cstdint>
#include <vector>
#include <string_view>
#include <random>

#include "fmt/format.h"

#include "txt/window.hpp"
#include "txt/image.hpp"
#include "txt/renderer.hpp"

#include "demos/app.hpp"
#include "demos/text_bounce.hpp"

static auto entry([[maybe_unused]]std::vector<std::string_view> const& args) -> void {
    auto window = txt::make_window({"Hello, Text!"});
    txt::renderer::init(window);
    auto sfmono = txt::renderer::instance()->load_font({
        .filename = "./res/fonts/SFMono/SFMono Regular Nerd Font Complete.otf",
        .size     = 11,
        .family   = "SF Mono Nerd Font",
        .style    = "Regular",
    });

    window->add_event_listener(txt::event_type::key_up, [&](auto const& event) {
        auto const& e = static_cast<txt::key_up_event const&>(event);
        if (e.keycode() == txt::keycode::Q) window->close();
    });

    txt::loop(window, [&] (double){
        txt::begin_frame();
        txt::viewport(0, 0, window->buffer_width(), window->buffer_height());
        txt::clear_color(0x000000);
        txt::clear();

        txt::text("Hello, World!", {});

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
