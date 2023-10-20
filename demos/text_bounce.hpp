#ifndef DEMOS_TEXT_BOUNCE_HPP
#define DEMOS_TEXT_BOUNCE_HPP
#include <random>

#include "app.hpp"
#include "glm/vec3.hpp"

#include "txt/window.hpp"
#include "txt/renderer.hpp"

class text_bounce : public demo::app {
public:
    text_bounce(txt::window_ref_t window)
        : app(window) {
        text_pos = {float(window->width()) / 2.0f, float(window->height()) / 2.0f};
        speed = 128.0f;
        scale = 2.0f;
        color = glm::vec4{1.0f};
        text_size = txt::text_size("Hello, World!", glm::vec2{scale});
    }

    auto update(double dt) -> void {
        std::uniform_int_distribution<std::mt19937::result_type> dist(0, 360);
        text_pos += text_heading * speed * float(dt);
        if (text_pos.x + (text_size.x + text_padding.x) / 2.0f > float(m_window->width())) {
            text_pos.x = float(m_window->width()) - (text_size.x + text_padding.x) / 2.0f;
            text_heading.x *= -1;
            color = txt::hsb2rgb(float(dist(rng)), 1.0f, 1.0f);
        } else if (text_pos.x - text_size.x / 2.0f < 0.0f) {
            text_pos.x = (text_size.x + text_padding.x) / 2.0f;
            text_heading.x *= -1;
            color = txt::hsb2rgb(float(dist(rng)), 1.0f, 1.0f);
        }
        if (text_pos.y + (text_size.y + text_padding.y) / 2.0f > float(m_window->height())) {
            text_pos.y = float(m_window->height()) - (text_size.y + text_padding.y) / 2.0f;
            text_heading.y *= -1;
            color = txt::hsb2rgb(float(dist(rng)), 1.0f, 1.0f);
        } else if (text_pos.y - text_size.y / 2.0f  < 0.0f) {
            text_pos.y = (text_size.y + text_padding.y) / 2.0f;
            text_heading.y *= -1;
            color = txt::hsb2rgb(float(dist(rng)), 1.0f, 1.0f);
        }

        txt::rect(text_pos, text_size + text_padding, 0.0f, {color, 1.0f});
        txt::text("Hello, World!", text_pos - text_size / 2.0f, {color * 0.25f, 1.0f}, glm::vec2{scale});
    }

private:
    glm::vec2 text_pos;
    glm::vec2 text_size;
    float     speed;
    float     scale;
    glm::vec3 color;
    glm::vec2 text_heading{1.0f};
    glm::vec2 text_padding{8.0f, 4.0f};

    std::random_device rdevice{};
    std::mt19937 rng{rdevice()};
};


#endif // DEMOS_TEXT_BOUNCE_HPP
