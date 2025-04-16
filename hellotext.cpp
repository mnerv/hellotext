#include <span>

#include "txt/utils.hpp"
#include "txt/window.hpp"
#include "txt/renderer.hpp"
#include "txt/text/fonts.hpp"

using namespace txt::types;

auto entry([[maybe_unused]]std::span<char const*> const& args) -> void {
    static auto win = txt::make_window({"Hello, Text!"});
    static auto ren = txt::make_renderer(win);
    // ren->load_font({
    //     "./res/fonts/Cozette/CozetteVector.ttf",
    //     128,
    //     txt::text_render_mode::raster,
    // });
    ren->load_font({
        "./res/fonts/RobotoMono/RobotoMonoNerdFontMono-Medium.ttf",
        16,
        txt::text_render_mode::raster,
        {0, 0xFFFF}
    });

    static auto mouse_x = 0.0f;
    static auto mouse_y = 0.0f;
    static auto rot = 0.0f;

    win->add_event_listener([&](txt::touch_start_event const& e) {
        if (e.points().empty()) return;
        mouse_x = f32(e.points()[0].x());
        mouse_y = f32(win->height() - e.points()[0].y());
        fmt::print("{}\n", e.str());
    });
    win->add_event_listener([&](txt::touch_move_event const& e) {
        if (e.points().empty()) return;
        mouse_x = f32(e.points()[0].x());
        mouse_y = f32(win->height() - e.points()[0].y());
        fmt::print("{} {}: {}={}\n", e.str(), mouse_y, win->height(), win->buffer_width());
    });
    win->add_event_listener([&](txt::mouse_move_event const& e) {
        fmt::print("{}\n", e.str());
    });
    win->add_event_listener([&](txt::key_down_event const& e) {
        fmt::print("{}\n", e.str());
    });
    win->add_event_listener([&](txt::key_up_event const& e) {
        if (e.keycode() == txt::keycode::Q) win->close();
        if (e.keycode() == txt::keycode::F) win->toggle_fullscreen();
    });

    txt::loop(win, [&] {
        rot += txt::pi_f32 / 180.0f * 3.0f;

        ren->begin();
        ren->viewport(0, 0, win->buffer_width(), win->buffer_height());
        ren->clear_color(0x000000);
        ren->clear();

        ren->text("Hello, World!", {0.0f, 0.0f});

        ren->rect(glm::vec2{mouse_x, mouse_y}, glm::vec2{1.0f, 100.0f},
                  rot, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, {});
        ren->rect(glm::vec2{mouse_x, mouse_y}, glm::vec2{1.0f, 100.0f},
                  rot+txt::pi_f32/2.0f, glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, {});

        ren->rect(glm::vec2{mouse_x, mouse_y}, glm::vec2{32.0f, 32.0f},
                  rot, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}, {});

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
