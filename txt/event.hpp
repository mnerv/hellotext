#ifndef TXT_EVENT_HPP
#define TXT_EVENT_HPP
#include <cstdint>
#include <string>
#include <array>
#include <chrono>

#include "fmt/format.h"
#include "utility.hpp"
#include "input.hpp"

namespace txt {
enum class event_category : std::uint16_t {
    none        = bit_on(0),
    application = bit_on(1),
    window      = bit_on(2),
    buffer      = bit_on(3),
    mouse       = bit_on(4),
    keyboard    = bit_on(5),
    controller  = bit_on(6),
    touch       = bit_on(7),
    custom      = bit_on(8),
};

enum class event_type : std::uint32_t {
    none = 0,
    // application
    drop, update, draw,
    // window
    window_resize, window_move, window_focus, window_icon, window_close,
    window_maximize,
    // buffer
    framebuffer_resize, content_scale,
    // mouse
    mouse_enter, mouse_leave,
    mouse_move,  mouse_down, mouse_up, mouse_wheel,
    // keyboard
    key_down, key_up, key_typed,
    // controller
    controller,
    // touch
    touch_start, touch_move, touch_end,
};

using event_clock = std::chrono::high_resolution_clock;
using event_time_point = std::chrono::high_resolution_clock::time_point;
inline constexpr auto event_time_point_ms(auto const& timepoint) {
    return std::chrono::time_point_cast<std::chrono::milliseconds>(timepoint).time_since_epoch().count();
}

class event {
public:
    [[nodiscard]]auto time_point() const -> event_time_point const& { return m_time_point; }
    [[nodiscard]]constexpr auto type() const -> event_type { return m_type; }
    [[nodiscard]]constexpr auto category() const -> event_category { return  m_category; }
    [[nodiscard]]virtual auto name() const -> char const* = 0;
    [[nodiscard]]virtual auto str() const -> std::string = 0;

protected:
    event(event_type const& type, event_category const& category)
        : m_time_point(event_clock::now())
        , m_type(type)
        , m_category(category) {}
    virtual ~event() = default;

protected:
    event_time_point m_time_point;
    event_type       m_type;
    event_category   m_category;
};

class drop_event : public event {
public:
    drop_event(std::vector<std::string> const& paths)
            : event(event_type::drop, event_category::application),
              m_paths(paths) {};

    [[nodiscard]]auto name() const -> char const* override {
        return "drop_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "size: " + std::to_string(m_paths.size()) + ", ";
        str += "paths: [ ";
        for (std::size_t i = 0; i < m_paths.size(); i++) {
            str += "\"" + m_paths.at(i) + "\"";
            if (i < m_paths.size() - 1) str += ", ";
        }
        str += " ] }";
        return str;
    }

    [[nodiscard]]auto size() const -> std::size_t { return m_paths.size(); }
    [[nodiscard]]auto paths() const -> std::vector<std::string> { return m_paths; }

private:
    std::vector<std::string> m_paths;
};

class update_event : public event {
public:
    update_event(double const& time, double const& delta)
            : event(event_type::update, event_category::application),
              m_time(time), m_delta(delta) {}

    [[nodiscard]]auto name() const -> char const* override { return "update_event"; }
    [[nodiscard]]auto str() const -> std::string override {
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "time: "  + std::to_string(m_time)  + ", ";
        str += "delta: " + std::to_string(m_delta) + " }";
        return str;
    }

    [[nodiscard]]auto time()  const -> double { return m_time;  }
    [[nodiscard]]auto delta() const -> double { return m_delta; }

private:
    double m_time;
    double m_delta;
};

class draw_event : public event {
public:
    draw_event(double const& time, double const& delta)
            : event(event_type::update, event_category::application),
              m_time(time), m_delta(delta) {}

    [[nodiscard]]auto name() const -> char const* override { return "draw_event"; }
    [[nodiscard]]auto str() const -> std::string override {
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "time: "  + std::to_string(m_time)  + ", ";
        str += "delta: " + std::to_string(m_delta) + " }";
        return str;
    }

    [[nodiscard]]auto time()  const -> double { return m_time;  }
    [[nodiscard]]auto delta() const -> double { return m_delta; }

private:
    double m_time;
    double m_delta;
};

class window_event : public event {
public:
    window_event(std::size_t const& id, event_type const& type)
        : event(type, event_category::window)
        , m_id(id) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "window_event";
    }
    auto id() const -> std::size_t { return m_id; }

protected:
    std::size_t m_id;
};

class window_resize_event : public window_event {
public:
    window_resize_event(std::size_t const& id, std::uint32_t const& width, std::uint32_t const& height)
            : window_event(id, event_type::window_resize),
              m_width(width), m_height(height) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "window_resize_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "width: "  + std::to_string(m_width)  + ", ";
        str += "height: " + std::to_string(m_height) + " }";
        return str;
    }

    [[nodiscard]]auto width() const -> std::uint32_t { return m_width; }
    [[nodiscard]]auto height() const -> std::uint32_t { return m_height; }

private:
    std::uint32_t m_width;
    std::uint32_t m_height;
};

class window_move_event : public window_event {
public:
    window_move_event(std::size_t const& id, std::int32_t const& x, std::int32_t const& y)
            : window_event(id, event_type::window_move),
              m_x(x), m_y(y) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "window_move_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "x: " + std::to_string(m_x) + ", ";
        str += "y: " + std::to_string(m_y) + " }";
        return str;
    }
    [[nodiscard]]auto x() const -> std::int32_t { return m_x; }
    [[nodiscard]]auto y() const -> std::int32_t { return m_y; }

private:
    std::int32_t m_x;
    std::int32_t m_y;
};

class window_focus_event : public window_event {
public:
    explicit window_focus_event(std::size_t const& id, bool const& is_focus)
            : window_event(id, event_type::window_focus)
            , m_focus(is_focus) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "window_focus_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "id: " + std::to_string(m_id) + ", ";
        str += "focus: ";
        str += (m_focus ? "true" : "false") + " }"s;
        return str;
    }

    [[nodiscard]]auto is_focus() const -> bool { return m_focus; }

private:
    bool m_focus;
};

class window_icon_event : public window_event {
public:
    explicit window_icon_event(std::size_t const& id, bool const& is_icon)
            : window_event(id, event_type::window_icon),
              m_is_icon(is_icon) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "window_icon_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "iconified: ";
        str += (m_is_icon ? "true" : "false") + " }"s;
        return str;
    }

    [[nodiscard]]auto is_icon() const -> bool { return m_is_icon; }

private:
    bool m_is_icon;
};

class window_close_event : public window_event {
public:
    explicit window_close_event(std::size_t const& id)
        : window_event(id, event_type::window_close) {}
    [[nodiscard]]auto name() const -> char const* override {
        return "window_close_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " {";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += " }";
        return str;
    }
};

class window_maximize_event : public window_event {
public:
    explicit window_maximize_event(std::size_t const& id, bool const& is_maximize)
            : window_event(id, event_type::window_maximize),
              m_is_maximize(is_maximize) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "window_maximize_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "maximized: ";
        str += (m_is_maximize ? "true" : "false") + " }"s;
        return str;
    }

    [[nodiscard]]auto is_maximize() const -> bool { return m_is_maximize; }

private:
    bool m_is_maximize;
};

class framebuffer_resize_event : public event {
public:
    framebuffer_resize_event(std::size_t const& id, std::uint32_t const& width, std::uint32_t const& height)
            : event(event_type::framebuffer_resize, event_category::window),
              m_id(id), m_width(width), m_height(height) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "framebuffer_resize_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "id: "     + std::to_string(m_id)  + ", ";
        str += "width: "  + std::to_string(m_width)  + ", ";
        str += "height: " + std::to_string(m_height) + " }";
        return str;
    }

    [[nodiscard]]auto id() const -> std::size_t { return m_id; }
    [[nodiscard]]auto width() const -> std::uint32_t { return m_width; }
    [[nodiscard]]auto height() const -> std::uint32_t { return m_height; }

private:
    std::size_t  m_id;
    std::uint32_t m_width;
    std::uint32_t m_height;
};

class content_scale_event : public event {
public:
    content_scale_event(std::size_t const& id, double const& x, double const& y)
            : event(event_type::content_scale, event_category::buffer),
              m_id(id), m_x(x), m_y(y) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "content_scale_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "x: " + std::to_string(m_x) + ",";
        str += "y: " + std::to_string(m_y) + " }";
        return str;
    }

    [[nodiscard]]auto id() const -> std::size_t { return m_id; }
    [[nodiscard]]auto x() const -> double { return m_x; }
    [[nodiscard]]auto y() const -> double { return m_y; }

private:
    std::size_t m_id;
    double      m_x;
    double      m_y;
};

class mouse_event : public event {
public:
    mouse_event(event_type const& type, double const& x, double const& y)
        : event(type, event_category::mouse), m_x(x), m_y(y) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "mouse_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "x: " + fmt::format("{:.3f}", m_x) + ", ";
        str += "y: " + fmt::format("{:.3f}", m_y) + " }";
        return str;
    }

    [[nodiscard]]auto x() const -> double { return m_x; }
    [[nodiscard]]auto y() const -> double { return m_y; }

protected:
    double m_x;
    double m_y;
};

class mouse_move_event : public mouse_event {
public:
    mouse_move_event(double const& x, double const& y)
        : mouse_event(event_type::mouse_move, x, y) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "mouse_move_event";
    }
};

class mouse_down_event : public mouse_event {
public:
    mouse_down_event(mouse_button const& button, modifier_flags const& modifiers,
                     double const& x, double const& y)
        : mouse_event(event_type::mouse_down, x, y),
          m_button(button), m_mods(modifiers) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "mouse_down_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "button: " + std::to_string(std::uint32_t(m_button)) + ", ";
        str += "mods: "   + fmt::format("{:#x}", m_mods.raw())      + ", ";
        str += "x: " + fmt::format("{:.3f}", m_x) + ", ";
        str += "y: " + fmt::format("{:.3f}", m_y) + " }";
        return str;
    }

    [[nodiscard]]auto button() const -> mouse_button { return m_button; }
    [[nodiscard]]auto modifiers() const -> modifier_flags const& { return m_mods; }

private:
    mouse_button   m_button;
    modifier_flags m_mods;
};

class mouse_up_event : public mouse_event {
public:
    mouse_up_event(mouse_button const& button, modifier_flags const& modifiers,
                        double const& x, double const& y)
            : mouse_event(event_type::mouse_up, x, y),
              m_button(button), m_mods(modifiers) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "mouse_up_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "button: " + std::to_string(std::uint32_t(m_button)) + ", ";
        str += "mods: "   + fmt::format("{:#x}", m_mods.raw())      + ", ";
        str += "x: " + fmt::format("{:.3f}", m_x) + ", ";
        str += "y: " + fmt::format("{:.3f}", m_y) + " }";
        return str;
    }

    [[nodiscard]]auto button() const -> mouse_button { return m_button; }
    [[nodiscard]]auto mods() const -> modifier_flags const& { return m_mods; }

protected:
    mouse_button   m_button;
    modifier_flags m_mods;
};

class mouse_wheel_event : public mouse_event {
public:
    mouse_wheel_event(double const& dx, double const& dy, double const& x, double const& y)
            : mouse_event(event_type::mouse_wheel, x, y),
              m_dx(dx), m_dy(dy) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "mouse_wheel_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "dx: "  + std::to_string(m_dx)  + ", ";
        str += "dy: " + std::to_string(m_dy) + ", ";
        str += "x: " + fmt::format("{:.3f}", m_x) + ", ";
        str += "y: " + fmt::format("{:.3f}", m_y) + " }";
        return str;
    }

    [[nodiscard]]auto dx() const -> double { return m_dx; }
    [[nodiscard]]auto dy() const -> double { return m_dy; }

private:
    double m_dx;
    double m_dy;
};

class mouse_enter_event : public mouse_event {
public:
    mouse_enter_event(double const& x, double const& y)
            : mouse_event(event_type::mouse_enter, x, y) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "mouse_enter_event";
    }
};

class mouse_leave_event : public mouse_event {
public:
    mouse_leave_event(double const& x, double const& y)
            : mouse_event(event_type::mouse_leave, x, y) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "mouse_leave_event";
    }
};

class key_event : public event {
public:
    key_event(event_type const& type, txt::keycode const& keycode, txt::scancode const& scancode, modifier_flags const& modifiers)
        : event(type, event_category::keyboard)
        , m_keycode(keycode)
        , m_scancode(scancode)
        , m_modifiers(modifiers) {}

    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += fmt::format(" {{ time: {} ms, keycode: {:#x}, scancode: {:#x}, modifier: {:#x} }}",
                            event_time_point_ms(m_time_point),
                            std::uint32_t(m_keycode),
                            std::uint16_t(m_scancode),
                            m_modifiers.raw());
        return str;
    }

    [[nodiscard]]auto keycode() const -> txt::keycode { return m_keycode; }
    [[nodiscard]]auto scancode() const -> txt::scancode { return m_scancode; }
    [[nodiscard]]auto modifiers() const -> modifier_flags const& { return m_modifiers; }

protected:
    txt::keycode   m_keycode;
    txt::scancode  m_scancode;
    modifier_flags m_modifiers;
};

class key_down_event : public key_event {
public:
    key_down_event(txt::keycode const& keycode, txt::scancode const& scancode, modifier_flags const& modifiers, bool const& is_repeat)
        : key_event(event_type::key_down, keycode, scancode, modifiers)
        , m_is_repeat(is_repeat) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "key_down_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += fmt::format(" {{ time: {} ms, keycode: {:#x}, scancode: {:#x}, modifier: {:#x}, repeat: {} }}",
                            event_time_point_ms(m_time_point),
                            std::uint32_t(m_keycode),
                            std::uint16_t(m_scancode),
                            m_modifiers.raw(),
                            m_is_repeat);
        return str;
    }
    auto is_repeat() const -> bool { return m_is_repeat; }

private:
    bool m_is_repeat;
};

class key_up_event : public key_event {
public:
    key_up_event(txt::keycode const& keycode, txt::scancode const& scancode, modifier_flags const& modifiers)
        : key_event(event_type::key_down, keycode, scancode, modifiers) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "key_up_event";
    }
};

class key_typed_event : public event {
public:
    key_typed_event(std::uint32_t const& codepoint)
        : event(event_type::key_typed, event_category::keyboard)
        , m_codepoint(codepoint) {}

    auto name() const -> char const* { return "key_typed_event"; }
    auto str() const -> std::string {
        std::string str{name()};
        str += "{ " + std::to_string(m_codepoint) + " }";
        return str;
    };

private:
    std::uint32_t m_codepoint;
};

class controller_event : public event {
public:
    controller_event(std::uint32_t const& id)
        : event(event_type::controller, event_category::controller)
        , m_id(id) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "controller_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "id: " + std::to_string(m_id) + " }";
        return str;
    }

private:
    std::uint32_t m_id;
};

class touch_point {
public:
    touch_point(std::size_t const& id = 0, double const& x = 0.0, double const& y = 0.0)
        : m_id(id), m_x(x), m_y(y) {}

    [[nodiscard]]auto str() const -> std::string {
        using namespace std::string_literals;
        std::string str{"touch_point {"};
        str += "id: " + std::to_string(m_id) + ", ";
        str += "x: "  + fmt::format("{:.3f}", m_x) + ", ";
        str += "y: "  + fmt::format("{:.3f}", m_y) + " }";
        return str;
    }
    [[nodiscard]]auto id() const -> std::size_t { return m_id; }
    [[nodiscard]]auto x() const -> double { return m_x; }
    [[nodiscard]]auto y() const -> double { return m_y; }

protected:
    std::size_t m_id;
    double m_x;
    double m_y;
};
inline constexpr std::size_t max_touch_points = 32;
using touch_points_t = std::array<touch_point, max_touch_points>;

class touch_event : public event {
public:
    touch_event(event_type const& type, std::size_t const& size, touch_points_t const& points)
        : event(type, event_category::touch)
        , m_size(size)
        , m_points(points) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "touch_event";
    }
    [[nodiscard]]auto str() const -> std::string override {
        using namespace std::string_literals;
        std::string str{name()};
        str += " { ";
        str += "time: " + std::to_string(event_time_point_ms(m_time_point)) + " ms, ";
        str += "size: " + std::to_string(m_size) + ", ";
        for (std::size_t i = 0; i < m_size; ++i) {
            str += "[" + std::to_string(i) + "]: " + m_points[i].str();
            if (i < m_points.size() - 1) str += ", ";
            else str += " }";
        }
        return str;
    }
    [[nodiscard]]auto size() const -> std::size_t { return m_size; }
    [[nodiscard]]auto points() const -> touch_points_t const& { return m_points; }

protected:
    std::size_t    m_size;
    touch_points_t m_points;
};

class touch_start_event : public touch_event {
public:
    touch_start_event(std::size_t const& size, touch_points_t const& points)
        : touch_event(event_type::touch_start, size, points) {}

    [[nodiscard]]auto name() const -> char const* override {
        return "touch_start_event";
    }
};

class touch_move_event : public touch_event {
public:
    touch_move_event(std::size_t const& size, touch_points_t const& points)
        : touch_event(event_type::touch_move, size, points) { }

    [[nodiscard]]auto name() const -> char const* override {
        return "touch_move_event";
    }
};

class touch_end_event : public touch_event {
public:
    touch_end_event(std::size_t const& size, touch_points_t const& points)
        : touch_event(event_type::touch_end, size, points) { }

    [[nodiscard]]auto name() const -> char const* override {
        return "touch_end_event";
    }
};

template <typename T>
inline consteval auto event_t_to_enum() -> event_type {
    if constexpr (std::is_same_v<T, drop_event>)
        return event_type::drop;
    else if constexpr (std::is_same_v<T, update_event>)
        return event_type::update;
    else if constexpr (std::is_same_v<T, draw_event>)
        return event_type::draw;
    else if constexpr (std::is_same_v<T, window_resize_event>)
        return event_type::window_resize;
    else if constexpr (std::is_same_v<T, window_move_event>)
        return event_type::window_move;
    else if constexpr (std::is_same_v<T, window_focus_event>)
        return event_type::window_focus;
    else if constexpr (std::is_same_v<T, window_icon_event>)
        return event_type::window_icon;
    else if constexpr (std::is_same_v<T, window_close_event>)
        return event_type::window_close;
    else if constexpr (std::is_same_v<T, window_maximize_event>)
        return event_type::window_maximize;
    else if constexpr (std::is_same_v<T, framebuffer_resize_event>)
        return event_type::framebuffer_resize;
    else if constexpr (std::is_same_v<T, content_scale_event>)
        return event_type::content_scale;
    else if constexpr (std::is_same_v<T, mouse_enter_event>)
        return event_type::mouse_enter;
    else if constexpr (std::is_same_v<T, mouse_leave_event>)
        return event_type::mouse_leave;
    else if constexpr (std::is_same_v<T, mouse_move_event>)
        return event_type::mouse_move;
    else if constexpr (std::is_same_v<T, mouse_down_event>)
        return event_type::mouse_down;
    else if constexpr (std::is_same_v<T, mouse_up_event>)
        return event_type::mouse_up;
    else if constexpr (std::is_same_v<T, mouse_wheel_event>)
        return event_type::mouse_wheel;
    else if constexpr (std::is_same_v<T, key_down_event>)
        return event_type::key_down;
    else if constexpr (std::is_same_v<T, key_up_event>)
        return event_type::key_up;
    else if constexpr (std::is_same_v<T, key_typed_event>)
        return event_type::key_typed;
    else if constexpr (std::is_same_v<T, controller_event>)
        return event_type::controller;
    else if constexpr (std::is_same_v<T, touch_start_event>)
        return event_type::touch_start;
    else if constexpr (std::is_same_v<T, touch_move_event>)
        return event_type::touch_move;
    else if constexpr (std::is_same_v<T, touch_end_event>)
        return event_type::touch_end;
    else
        static_assert(false, "Unrecognized event type in etype_to_enum");
}
}

#endif  // TXT_EVENT_HPP

