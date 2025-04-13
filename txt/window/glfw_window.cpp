#include "window.hpp"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad/glad.h"

namespace txt {
static auto setup_opengl() -> void {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

static constexpr auto convert_glfw_keycode(i32 value) -> txt::keycode {
    return txt::keycode(value);
}

static auto convert_glfw_scancode(i32 value) -> txt::scancode {
    txt::scancode scan = txt::scancode::reserved;
    if (value >= 'A' && value <= 'Z')
        scan = txt::scancode(std::uint16_t(txt::scancode::a) + (value - 'A'));
    if (value >= 290 && value <= 301 )
        scan = txt::scancode(std::uint16_t(txt::scancode::f1) + (value - 290));
    return scan;
};

auto window::fullscreen_native() -> void {
    m_windowed.m_width = m_width;
    m_windowed.m_height = m_height;
    m_windowed.m_position_x = m_position_x;
    m_windowed.m_position_y = m_position_y;

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor) return;

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (!mode) return;

    glfwSetWindowMonitor(
        static_cast<GLFWwindow*>(m_native), 
        monitor,
        0, 0,
        mode->width,
        mode->height,
        mode->refreshRate
    );

    m_is_fullscreen = true;
}

auto window::exit_fullscreen_native() -> void {
    glfwSetWindowMonitor(
        static_cast<GLFWwindow*>(m_native),
        nullptr,
        m_windowed.m_position_x,
        m_windowed.m_position_y,
        i32(m_windowed.m_width),
        i32(m_windowed.m_height),
        0
    );
    m_is_fullscreen = false;
}

auto window::setup_native() -> void {
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error(fmt::format("Failed to initialize GLFW\n"));
    setup_opengl();

    if (m_native == nullptr)
        m_native = glfwCreateWindow(i32(m_width), i32(m_height), m_title.c_str(), nullptr, nullptr);
    if (m_native == nullptr)
        throw std::runtime_error(fmt::format("Failed to create GLFW window\n"));
    glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_native));

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error(fmt::format("Failed to initialize GLAD\n"));

    glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_native), this);
    glfwSetWindowCloseCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr) {
        auto ptr = reinterpret_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_should_close = true;
        auto const e = window_close_event(0);

        auto const it = ptr->m_listeners.find(event_type::window_close);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetWindowSizeCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 width, i32 height) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_width  = u32(width);
        ptr->m_height = u32(height);
        auto const e = window_resize_event(0, ptr->m_width, ptr->m_height);

        auto const it = ptr->m_listeners.find(event_type::window_resize);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 width, i32 height) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_buffer_width  = u32(width);
        ptr->m_buffer_height = u32(height);
        auto const e = framebuffer_resize_event(0, ptr->m_buffer_width, ptr->m_buffer_height);

        auto const it = ptr->m_listeners.find(event_type::framebuffer_resize);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetWindowPosCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 xpos, i32 ypos) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_position_x = xpos;
        ptr->m_position_y = ypos;
        auto const e = window_move_event(0,  ptr->m_position_x, ptr->m_position_y);

        auto const it = ptr->m_listeners.find(event_type::window_move);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetWindowFocusCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 focused) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_is_focused = bool(focused);
        auto const e = window_focus_event(0, ptr->m_is_focused);

        auto const it = ptr->m_listeners.find(event_type::window_focus);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    // glfwSetWindowIconifyCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 iconified) {
    //     auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
    //     (void)ptr;
    //     (void)iconified;
    // });
    glfwSetWindowMaximizeCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 maximized) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_is_fullscreen = bool(maximized);
        auto const e = window_maximize_event(0, ptr->m_is_fullscreen);

        auto const it = ptr->m_listeners.find(event_type::window_maximize);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetWindowContentScaleCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, float xscale, float yscale) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_content_scale_x = f64(xscale);
        ptr->m_content_scale_y = f64(yscale);
        auto const e = content_scale_event(0, ptr->m_content_scale_x, ptr->m_content_scale_y);

        auto const it = ptr->m_listeners.find(event_type::content_scale);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetCursorPosCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, f64 xpos, f64 ypos) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_mouse_x = xpos;
        ptr->m_mouse_y = ypos;
        auto const e = mouse_move_event(ptr->m_mouse_x, ptr->m_mouse_y);

        auto const it = ptr->m_listeners.find(event_type::mouse_move);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetCursorEnterCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 entered) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        glfwGetCursorPos(window_ptr, &ptr->m_mouse_x, &ptr->m_mouse_y);
        ptr->m_is_hovered = bool(entered);
        auto const e = mouse_enter_event(ptr->m_mouse_x, ptr->m_mouse_y);

        auto const it = ptr->m_listeners.find(event_type::mouse_enter);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetMouseButtonCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 button, i32 action, i32 mods) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        glfwGetCursorPos(window_ptr, &ptr->m_mouse_x, &ptr->m_mouse_y);
        if (action == GLFW_PRESS) {
            auto const e = mouse_down_event(mouse_button(button), modifier_flags{u32(mods)}, ptr->m_mouse_x, ptr->m_mouse_y);
            auto const it = ptr->m_listeners.find(event_type::mouse_down);
            if (it == std::end(ptr->m_listeners)) return;
            auto const& fns = it->second;
            std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                fn.second(e);
            });
        } else {
            auto const e = mouse_up_event(mouse_button(button), modifier_flags{u32(mods)}, ptr->m_mouse_x, ptr->m_mouse_y);
            auto const it = ptr->m_listeners.find(event_type::mouse_up);
            if (it == std::end(ptr->m_listeners)) return;
            auto const& fns = it->second;
            std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                fn.second(e);
            });
        }
    });
    glfwSetScrollCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, f64 xoffset, f64 yoffset) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        glfwGetCursorPos(window_ptr, &ptr->m_mouse_x, &ptr->m_mouse_y);
        auto const e = mouse_wheel_event(xoffset, yoffset, ptr->m_mouse_x, ptr->m_mouse_y);
        auto const it = ptr->m_listeners.find(event_type::mouse_wheel);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetKeyCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 key, [[maybe_unused]]i32 scancode, i32 action, i32 mods) {
        [[maybe_unused]]auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));

        if (action != GLFW_RELEASE) {
            auto const e = key_down_event(convert_glfw_keycode(key), convert_glfw_scancode(key), {u32(mods)}, action == GLFW_REPEAT);
            auto const it = ptr->m_listeners.find(event_type::key_down);
            if (it == std::end(ptr->m_listeners)) return;
            auto const& fns = it->second;
            std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                fn.second(e);
            });
        } else {
            auto const e = key_up_event(convert_glfw_keycode(key), convert_glfw_scancode(key), {u32(mods)});
            auto const it = ptr->m_listeners.find(event_type::key_up);
            if (it == std::end(ptr->m_listeners)) return;
            auto const& fns = it->second;
            std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                fn.second(e);
            });
        }
    });
    glfwSetCharCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, u32 codepoint) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        auto const e = key_typed_event(codepoint);
        auto const it = ptr->m_listeners.find(event_type::key_typed);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetDropCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, i32 count, char const** paths) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        auto const e = drop_event({paths, std::next(paths, count)});
        auto const it = ptr->m_listeners.find(event_type::drop);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });

    i32 width, height;
    glfwGetWindowSize(static_cast<GLFWwindow*>(m_native), &width, &height);
    m_width  = u32(width);
    m_height = u32(height);
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_native), &width, &height);
    m_buffer_width  = u32(width);
    m_buffer_height = u32(height);
    f32 content_scale_x, content_scale_y;
    glfwGetWindowContentScale(static_cast<GLFWwindow*>(m_native), &content_scale_x, &content_scale_y);
    m_content_scale_x = f64(content_scale_x);
    m_content_scale_x = f64(content_scale_y);
    i32 xpos, ypos;
    glfwGetWindowPos(static_cast<GLFWwindow*>(m_native), &xpos, &ypos);
    m_position_x = xpos;
    m_position_y = ypos;
}
auto window::clean_native() -> void {
    m_asio.stop();
    glfwDestroyWindow(static_cast<GLFWwindow*>(m_native));
    glfwTerminate();
    m_native = nullptr;
}

auto window::poll_native() -> void {
    glfwPollEvents();
}

auto window::swap_native() -> void {
    glfwSwapBuffers(static_cast<GLFWwindow*>(m_native));
}
}
