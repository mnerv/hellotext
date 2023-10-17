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
    // auto roboto = txt::renderer::instance()->load_font({
    //     .filename = "./res/fonts/RobotoMono/RobotoMonoNerdFontMono-Medium.ttf",
    //     .size     = 13,
    //     .family   = "Roboto Mono Nerd Font Mono",
    //     .style    = "Medium"
    // });
    // auto sfmono = txt::renderer::instance()->load_font({
    //     .filename = "res/fonts/SFMono/SFMono Semibold Nerd Font Complete.otf",
    //     .size     = 11,
    //     .family   = "SF Mono",
    //     .style    = "Semibold"
    // });

    // auto texture = txt::make_texture(txt::load_image_rgba("./nurture.jpg"));

    double rotation = 0.0;
    txt::loop(window, [&] (double dt){
        txt::begin_frame();
        txt::viewport(0, 0, window->buffer_width(), window->buffer_height());
        txt::clear_color(0x000000);
        txt::clear();

        rotation += 2.0 * txt::pi * 0.25 * dt;

        glm::vec2 text_pos = {float(window->width()) / 2.0f, float(window->height()) / 2.0f};
        txt::rect(text_pos, {100.0f, 100.0f}, float(rotation));
        txt::text("Hello, World!", {text_pos.x, text_pos.y}, glm::vec4{1.0f});
        // txt::text("Oh no!", {float(window->width()) / 2.0f, float(window->height()) / 2.0f + 16.0f}, glm::vec4{1.0f, 1.0f, 0.0f, 1.0f}, roboto);
        // txt::text("wow", {float(window->width()) / 2.0f, float(window->height()) / 2.0f + 32.0f}, glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}, sfmono);

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
