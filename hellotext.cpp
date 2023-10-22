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

    std::vector<std::string> texts{};
    auto push_text = [&](std::string const& str) {
        texts.push_back(str);
        if (texts.size() > 100) texts.erase(std::begin(texts));
    };

    // auto mouse_event = [&](txt::event const& e) {
    //     push_text(e.str());
    // };
    // window->add_event_listener(txt::event_type::mouse_move, mouse_event);
    // window->add_event_listener(txt::event_type::mouse_wheel, mouse_event);
    // window->add_event_listener(txt::event_type::mouse_down, mouse_event);

    auto callback = [&](txt::event const& e) {
        push_text(e.str());
    };
    window->add_event_listener(txt::event_type::key_down, callback);
    window->add_event_listener(txt::event_type::key_up, callback);
    auto keydown = [&](txt::event const& ev) {
        auto const& e = static_cast<txt::key_down_event const&>(ev);
        if (e.keycode() == txt::keycode::Q)
            window->close();
    };
    window->add_event_listener(txt::event_type::key_down, keydown);

    txt::loop(window, [&] (double dt){
        txt::begin_frame();
        txt::viewport(0, 0, window->buffer_width(), window->buffer_height());
        txt::clear_color(0x000000);
        txt::clear();

        glm::vec2 const text_scale{1.0f};
        auto text = fmt::format("{:.3f} ms", dt * 1000.0f);
        auto text_size = txt::text_size(text, text_scale);
        txt::text(text, {float(window->width()) - text_size.x - 4.0f, float(window->height()) - text_size.y - 4.0f}, glm::vec4{1.0f}, text_scale);

        float offset_y = 0.0f;
        for (auto const& txt : texts) {
            txt::text(txt, {4.0f, float(window->height()) - offset_y - text_size.y - 4.0f}, glm::vec4{txt::hsb2rgb(offset_y / float(window->height()) * 360.0f, 1.0f, 1.0f), 1.0f}, text_scale);
            offset_y += text_size.y;
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
