#include <cstdint>
#include <vector>
#include <string_view>
#include <random>

#include "fmt/format.h"

#include "txt/window.hpp"
#include "txt/image.hpp"
#include "txt/renderer.hpp"

/**
 * Convert HSB value to RGB.
 * @param hue        - Hue value with range [0, 360]
 * @param saturation - Saturation value with range [0, 1]
 * @param brightness - Brightness value with range [0, 1]
 * @return RGB with range [0, 1]
*/
auto hsb2rgb(float hue, float saturation, float brightness) -> glm::vec3 {
    auto const h = hue / 360.0f;
    auto rgb = glm::clamp(glm::abs(glm::mod(h * 6.0f + glm::vec3(0.0f, 4.0f, 2.0f), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f);
    rgb = rgb * rgb * (3.0f - 2.0f * rgb);
    return brightness * glm::mix(glm::vec3(1.0f), rgb, saturation);
}

static auto entry([[maybe_unused]]std::vector<std::string_view> const& args) -> void {
    auto window = txt::make_window({"Hello, Text!"});
    txt::renderer::init(window);
    auto roboto = txt::renderer::instance()->load_font({
        .filename = "./res/fonts/RobotoMono/RobotoMonoNerdFontMono-Regular.ttf",
        .size     = 27,
        .family   = "Roboto Mono Nerd Font Mono",
        .style    = "Regular"
    });
    // auto sfmono = txt::renderer::instance()->load_font({
    //     .filename = "res/fonts/SFMono/SFMono Regular Nerd Font Complete.otf",
    //     .size     = 14,
    //     .family   = "SF Mono",
    //     .style    = "Regular"
    // });

    // auto texture = txt::make_texture(txt::load_image_rgba("./nurture.jpg"));

    glm::vec2 text_pos = {float(window->width()) / 2.0f, float(window->height()) / 2.0f};
    glm::vec2 text_heading{1.0f};
    // float const speed = 0.0f;
    float const speed = 200.0f;
    float const scale = 1.0f;
    auto const txt_size = txt::text_size("Hello, World!", glm::vec2{scale}, roboto);
    glm::vec2 const text_padding{8.0f, 4.0f};
    glm::vec3 color{1.0f};

    std::random_device rdevice;
    std::mt19937 rng{rdevice()};
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, 360);

    txt::loop(window, [&] (double dt){
        txt::begin_frame();
        txt::viewport(0, 0, window->buffer_width(), window->buffer_height());
        txt::clear_color(0x000000);
        txt::clear();

        text_pos += text_heading * speed * float(dt);
        if (text_pos.x + (txt_size.x + text_padding.x) / 2.0f > float(window->width())) {
            text_pos.x = float(window->width()) - (txt_size.x + text_padding.x) / 2.0f;
            text_heading.x *= -1;
            color = hsb2rgb(float(dist(rng)), 1.0f, 1.0f);
        } else if (text_pos.x - txt_size.x / 2.0f < 0.0f) {
            text_pos.x = (txt_size.x + text_padding.x) / 2.0f;
            text_heading.x *= -1;
            color = hsb2rgb(float(dist(rng)), 1.0f, 1.0f);
        }
        if (text_pos.y + (txt_size.y + text_padding.y) / 2.0f > float(window->height())) {
            text_pos.y = float(window->height()) - (txt_size.y + text_padding.y) / 2.0f;
            text_heading.y *= -1;
            color = hsb2rgb(float(dist(rng)), 1.0f, 1.0f);
        } else if (text_pos.y - txt_size.y / 2.0f  < 0.0f) {
            text_pos.y = (txt_size.y + text_padding.y) / 2.0f;
            text_heading.y *= -1;
            color = hsb2rgb(float(dist(rng)), 1.0f, 1.0f);
        }

        txt::rect(text_pos, txt_size + text_padding, 0.0f, {color, 1.0f});
        txt::text("Hello, World!", text_pos - txt_size / 2.0f, {color * 0.25f, 1.0f}, glm::vec2{scale}, roboto);
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
