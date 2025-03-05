#include <cstdint>
#include <vector>
#include <string_view>
#include <random>
#include <tuple>

#include "fmt/format.h"

#include "txt/window.hpp"
#include "txt/image.hpp"
#include "txt/renderer.hpp"
#include "txt/fonts.hpp"

static auto entry([[maybe_unused]]std::vector<std::string_view> const& args) -> void {
    auto win = txt::make_window({"Hello, Text!"});
    auto rdr = txt::renderer::init(win);
    rdr->load_font({
        "./res/fonts/Cozette/CozetteVector.ttf",
        13,
        txt::text_render_mode::raster,
    });

    win->add_event_listener([&](txt::mouse_move_event const& e) {
        fmt::print("{}\n", e.str());
    });
    win->add_event_listener([&](txt::key_down_event const& e) {
        fmt::print("{}\n", e.str());
    });
    win->add_event_listener([&](txt::key_up_event const& e) {
        if (e.keycode() == txt::keycode::Q) win->close();
        if (e.keycode() == txt::keycode::F) win->fullscreen();
    });

    while (!win->should_close()) {
        rdr->begin();
        rdr->viewport(0, 0, win->buffer_width(), win->buffer_height());
        rdr->clear_color(0x000000);
        rdr->clear();

        rdr->text("Hello, World!", {0.0f, 0.0f});

        rdr->end();

        win->swap();
        win->poll();
    }
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
