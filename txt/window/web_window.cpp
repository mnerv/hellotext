#include "window.hpp"

#include "emscripten.h"
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#define EGL_EGLEXT_PROTOTYPES 1
#include "GL/gl.h"

namespace txt {
auto window::poll_native() -> void {
    double width, height;
    emscripten_get_element_css_size("canvas", &width, &height);
    if (u32(width) != m_width || u32(height) != m_height) {
        m_width         = u32(width);
        m_height        = u32(height);
        m_buffer_width  = u32(width * m_content_scale_x);
        m_buffer_height = u32(height * m_content_scale_y);
        emscripten_set_canvas_element_size("#canvas", m_buffer_width, m_buffer_height);

        {
            auto const e = framebuffer_resize_event(0, m_buffer_width, m_buffer_height);
            auto const it = m_listeners.find(e.type());
            if (it != std::end(m_listeners)) {
                auto const& fns = it->second;
                std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                    asio::post(m_asio, std::bind(fn.second, e));
                });
            }
        }
        {
            auto const e = window_resize_event(0, m_width, m_height);
            auto const it = m_listeners.find(e.type());
            if (it != std::end(m_listeners)) {
                auto const& fns = it->second;
                std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                    asio::post(m_asio, std::bind(fn.second, e));
                });
            }
        }
    }
    m_asio.poll();
}

auto window::swap_native() -> void {
    emscripten_webgl_commit_frame();
}

auto window::fullscreen_native() -> void {
}

auto window::exit_fullscreen_native() -> void {
}

auto window::setup_native() -> void {
    static auto const target_name = "#canvas";
    emscripten_set_window_title(m_title.c_str());
    emscripten_set_canvas_element_size(target_name, i32(m_width), i32(m_height));
    auto const device_pixel_ratio = emscripten_get_device_pixel_ratio();
    m_content_scale_x = device_pixel_ratio;
    m_content_scale_y = device_pixel_ratio;

    // Create WebGL context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    // attrs.explicitSwapControl = true;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    auto* context = new u32();
    *context = emscripten_webgl_create_context(target_name, &attrs);
    [[maybe_unused]]auto res = emscripten_webgl_make_context_current(*context);
    // TODO: Check make contet result if it was successful
    m_native = context;

    emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenFocusEvent const*, void* userData) {
        [[maybe_unused]]auto ptr = static_cast<window*>(userData);
        return EM_TRUE;
    });
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenFocusEvent const*, void* userData) {
        [[maybe_unused]]auto ptr = static_cast<window*>(userData);
        return EM_TRUE;
    });
    emscripten_set_mousemove_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenMouseEvent const* mouseEvent, void *userData) {
        auto ptr = static_cast<window*>(userData);
        auto const x = double(mouseEvent->clientX);
        auto const y = double(mouseEvent->clientY);
        ptr->m_mouse_x = x;
        ptr->m_mouse_y = y;
        auto it = ptr->m_listeners.find(event_type::mouse_move);
        if (it == ptr->m_listeners.end()) return EM_FALSE;
        auto const& fns = it->second;
        auto const event = mouse_move_event(x, y);
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            asio::post(ptr->m_asio, std::bind(fn.second, event));
        });
        return EM_TRUE;
    });
    emscripten_set_mousedown_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenMouseEvent const* mouseEvent, void *userData) {
        auto ptr = static_cast<window*>(userData);
        ptr->m_mouse_x = double(mouseEvent->clientX);
        ptr->m_mouse_y = double(mouseEvent->clientY);

        auto it = ptr->m_listeners.find(event_type::mouse_down);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;

        auto const button = mouse_button(mouseEvent->button);
        modifier_flags const mods{0};  // FIXME: Add modifiers: https://emscripten.org/docs/api_reference/html5.h.html#c.EmscriptenMouseEvent
        auto event = mouse_down_event(button, mods, ptr->m_mouse_x, ptr->m_mouse_y);
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            asio::post(ptr->m_asio, std::bind(fn.second, event));
        });
        return EM_TRUE;
    });
    emscripten_set_mouseup_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenMouseEvent const* mouseEvent, void *userData) {
        auto ptr = static_cast<window*>(userData);
        ptr->m_mouse_x = double(mouseEvent->clientX);
        ptr->m_mouse_y = double(mouseEvent->clientY);

        auto it = ptr->m_listeners.find(event_type::mouse_up);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;

        auto const button = mouse_button(mouseEvent->button);
        modifier_flags const mods{0};  // FIXME: Add modifiers: https://emscripten.org/docs/api_reference/html5.h.html#c.EmscriptenMouseEvent
        auto event = mouse_up_event(button, mods, ptr->m_mouse_x, ptr->m_mouse_y);
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            asio::post(ptr->m_asio, std::bind(fn.second, event));
        });
        return EM_TRUE;
    });
    emscripten_set_wheel_callback(target_name, this, EM_FALSE,
    [](int, EmscriptenWheelEvent const* wheelEvent, void *userData) {
        auto ptr = static_cast<window*>(userData);

        auto it = ptr->m_listeners.find(event_type::mouse_wheel);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;

        auto const e = mouse_wheel_event(wheelEvent->deltaX, wheelEvent->deltaY, ptr->m_mouse_x, ptr->m_mouse_y);
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            asio::post(ptr->m_asio, std::bind(fn.second, e));
        });
        return EM_FALSE;
    });
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenKeyboardEvent const* keyEvent, void *userData) {
        [[maybe_unused]]auto ptr = static_cast<window*>(userData);
        // TODO: Handle key down
        return EM_FALSE;
    });
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenKeyboardEvent const* keyEvent, void *userData) {
        [[maybe_unused]]auto ptr = static_cast<window*>(userData);
        // TODO: Handle key up
        return EM_FALSE;
    });
    // Touch Events
    emscripten_set_touchstart_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenTouchEvent const* touchEvent, void* userData) {
        auto ptr = static_cast<window*>(userData);
        auto const size = std::size_t(touchEvent->numTouches);
        auto const touches = touchEvent->touches;

        auto it = ptr->m_listeners.find(event_type::touch_start);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        touch_points_t points{};
        for (std::size_t i = 0; i < size; ++i) {
            points[i] = touch_point(std::size_t(touches[i].identifier), double(touches[i].clientX), double(touches[i].clientY));
        }

        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            asio::post(ptr->m_asio, std::bind(fn.second, touch_start_event(size, points)));
        });
        return EM_TRUE;
    });
    emscripten_set_touchmove_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenTouchEvent const* touchEvent, void* userData) {
        auto ptr = static_cast<window*>(userData);
        auto const size = std::size_t(touchEvent->numTouches);
        auto const touches = touchEvent->touches;

        auto it = ptr->m_listeners.find(event_type::touch_move);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        touch_points_t points{};
        for (std::size_t i = 0; i < size; ++i) {
            points[i] = touch_point(std::size_t(touches[i].identifier), double(touches[i].clientX), double(touches[i].clientY));
        }
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            asio::post(ptr->m_asio, std::bind(fn.second, touch_move_event(size, points)));
        });
        return EM_TRUE;
    });
    emscripten_set_touchend_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenTouchEvent const* touchEvent, void* userData) {
        auto ptr = static_cast<window*>(userData);
        auto const size = std::size_t(touchEvent->numTouches);
        auto const touches = touchEvent->touches;

        auto it = ptr->m_listeners.find(event_type::touch_end);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        touch_points_t points{};
        for (std::size_t i = 0; i < size; ++i) {
            points[i] = touch_point(std::size_t(touches[i].identifier), double(touches[i].clientX), double(touches[i].clientY));
        }
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            asio::post(ptr->m_asio, std::bind(fn.second, touch_end_event(size, points)));
        });
        return EM_TRUE;
    });
    emscripten_set_touchcancel_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenTouchEvent const* touchEvent, void* userData) {
        [[maybe_unused]]auto ptr = static_cast<window*>(userData);
        return EM_TRUE;
    });
}
auto window::clean_native() -> void {
    // TODO: Emscripten cleanup
    delete static_cast<int*>(m_native);
}

}
