#include <cstdint>
#include <vector>
#include <string_view>
#include <random>
#include <tuple>

#include "fmt/format.h"

#include "txt/window.hpp"
#include "txt/image.hpp"
#include "txt/renderer.hpp"

auto fn_ptr(int) -> void {}

auto test_event(txt::mouse_move_event const& e) -> void {
    fmt::print("fn ptr: {}\n", e.str());
}

static auto entry([[maybe_unused]]std::vector<std::string_view> const& args) -> void {
    auto window = txt::make_window({"Hello, Text!"});
    window->setup();
    txt::renderer::init(window);

    auto fn = [&](int) {};
    static_assert(std::is_same_v<txt::arg1_t<decltype(fn)>, int>);
    static_assert(std::is_same_v<txt::arg1_t<decltype(&fn_ptr)>, int>);

    auto mouse_move = [&](txt::mouse_move_event const& e) {
        fmt::print("mouse move: {}\n", e.str());
    };
    window->add_event_listener(mouse_move);
    window->add_event_listener(test_event);

    window->add_event_listener([&](txt::key_up_event const& e) {
        fmt::print("{}\n", e.str());
        if (e.keycode() == txt::keycode::Q) window->close();
        if (e.keycode() == txt::keycode::R) {
            fmt::print("remove\n");
            window->remove_event_listener(mouse_move);
        }
    });

    txt::loop(window, [&] (double){
        txt::begin_frame();
        txt::viewport(0, 0, window->buffer_width(), window->buffer_height());
        txt::clear_color(0x000000);
        txt::clear();

        txt::rect({125.0f, 125.0f}, {100.0f, 50.0f});
        txt::rect({125.0f, 125.0f}, {2.0f, 2.0f}, 0.0f, {1.0f, 0.0f, 0.0f, 1.0f});

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
