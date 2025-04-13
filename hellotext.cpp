#include <span>

#include "txt/window.hpp"
#include "txt/renderer.hpp"
#include "txt/text/fonts.hpp"

using namespace txt::types;

static auto entry([[maybe_unused]]std::span<char const*> const& args) -> void {
    fmt::print("hello\n");
    auto win = txt::make_window({"Hello, Text!"});
    auto ren = txt::make_renderer(win);
    ren->load_font({
        "./res/fonts/Cozette/CozetteVector.ttf",
        13,
        txt::text_render_mode::raster,
    });

    auto mouse_x = 0.0f;
    auto mouse_y = 0.0f;

    win->add_event_listener([&](txt::mouse_move_event const& e) {
        fmt::print("{}\n", e.str());
        mouse_x = f32(e.x());
        mouse_y = f32(win->buffer_height() - e.y());
    });
    win->add_event_listener([&](txt::key_down_event const& e) {
        fmt::print("{}\n", e.str());
    });
    win->add_event_listener([&](txt::key_up_event const& e) {
        if (e.keycode() == txt::keycode::Q) win->close();
        if (e.keycode() == txt::keycode::F) win->toggle_fullscreen();
    });

    txt::loop(win, [&] {
        ren->begin();
        ren->viewport(0, 0, win->buffer_width(), win->buffer_height());
        ren->clear_color(0x000000);
        ren->clear();

        ren->text("Hello, World!", {0.0f, 0.0f});

        ren->end();

        win->swap();
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
