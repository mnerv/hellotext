#include "window.hpp"
#include <stdexcept>
#include <fstream>
#include <algorithm>

#include "fmt/format.h"

#ifndef __EMSCRIPTEN__
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#else
#include "emscripten.h"
#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#include "GL/gl.h"

static auto const TARGET_NAME = "#canvas";
#endif

namespace txt {
auto make_window(window::props const& props) -> window_ref_t {
    return make_ref<window>(props);
}

auto read_text(std::filesystem::path const& filename) -> std::string {
    if (!std::filesystem::exists(filename))
        throw std::runtime_error(fmt::format("File \"{:s}\" does not exist!", filename.string()));
    std::ifstream input{filename, std::ios::in};
    if (!input.is_open() || input.fail())
        throw std::runtime_error(fmt::format("Failed to load \"{:s}\" text file!", filename.string()));
    return {
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };
}

[[nodiscard]]static auto info_opengl() -> std::string {
    return fmt::format(R"(Graphics API Info:
Vendor:   {:s}
Renderer: {:s}
Version:  {:s}
Shader:   {:s})",
        reinterpret_cast<char const*>(glGetString(GL_VENDOR)),
        reinterpret_cast<char const*>(glGetString(GL_RENDERER)),
        reinterpret_cast<char const*>(glGetString(GL_VERSION)),
        reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION))
    );
}

window::window(window::props const& properties)
    : m_title(properties.title)
    , m_width(properties.width)
    , m_height(properties.height)
    , m_buffer_width(properties.width)
    , m_buffer_height(properties.height) {
    setup_native();
    fmt::print("{}\n", info_opengl());
}
window::~window() {
    clean_native();
}

auto window::width() const noexcept -> std::uint32_t { return m_width; }
auto window::height() const noexcept -> std::uint32_t { return m_height; }
auto window::buffer_width() const noexcept -> std::uint32_t { return m_buffer_width; }
auto window::buffer_height() const noexcept -> std::uint32_t { return m_buffer_height; }
auto window::should_close() const noexcept -> bool { return m_should_close; }
auto window::x() const -> double { return m_position_x; }
auto window::y() const -> double { return m_position_y; }
auto window::content_scale_x() const -> double { return m_content_scale_x; }
auto window::content_scale_y() const -> double { return m_content_scale_y; }
auto window::is_focused() const -> bool { return m_is_focused; }
auto window::is_hovered() const -> bool { return m_is_hovered; }
auto window::is_maximized() const -> bool { return m_is_maximized; }

auto window::time() const -> double {
    auto const t = std::chrono::system_clock::now();
    auto const s = std::chrono::duration<double>(t.time_since_epoch());
    return s.count();
}
auto window::stopwatch() const -> double {
    auto const t = std::chrono::high_resolution_clock::now();
    auto const s = std::chrono::duration<double>(t.time_since_epoch());
    return s.count();
}

auto window::close() -> void {
    m_should_close = true;
}
auto window::poll() -> void {
#ifndef __EMSCRIPTEN__
    glfwPollEvents();
#else
#endif  // __EMSCRIPTEN__
}
auto window::swap() -> void {
#ifndef __EMSCRIPTEN__
    glfwSwapBuffers(static_cast<GLFWwindow*>(m_native));
#else
    emscripten_webgl_commit_frame();
#endif  // __EMSCRIPTEN__
}

auto window::add_event_listener(event_type const& type, event_fn const& fn) -> void {
    auto const id = fn.target_type().hash_code();
    if (m_listeners.find(type) == std::end(m_listeners)) {
        event_map fn_map{{id, fn}};
        m_listeners.insert({type, fn_map});
    } else {
        m_listeners[type].insert({id, fn});
    }
}
auto window::remove_event_listener(event_type const& type, event_fn const& fn) -> void {
    auto const id = fn.target_type().hash_code();
    auto fns = m_listeners.find(type);
    if (fns != std::end(m_listeners)) {
        fns->second.erase(id);
    }
}

#ifndef __EMSCRIPTEN__
static auto setup_opengl() -> void {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

static constexpr auto convert_glfw_keycode(std::int32_t value) -> txt::keycode {
    return txt::keycode(value);
}
static auto convert_glfw_scancode(std::int32_t value) -> txt::scancode {
    txt::scancode scan = txt::scancode::reserved;
    if (value >= 'A' && value <= 'Z') {
        scan = txt::scancode(std::uint16_t(txt::scancode::a) + (value - 'A'));
    }
    return scan;
};

auto window::setup_native() -> void {
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error(fmt::format("Failed to initialize GLFW\n"));
    setup_opengl();

    m_native = glfwCreateWindow(std::int32_t(m_width), std::int32_t(m_height), m_title.c_str(), nullptr, nullptr);
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
    glfwSetWindowSizeCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t width, std::int32_t height) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_width  = std::uint32_t(width);
        ptr->m_height = std::uint32_t(height);
        auto const e = window_resize_event(0, ptr->m_width, ptr->m_height);

        auto const it = ptr->m_listeners.find(event_type::window_resize);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t width, std::int32_t height) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_buffer_width  = std::uint32_t(width);
        ptr->m_buffer_height = std::uint32_t(height);
        auto const e = framebuffer_resize_event(0, ptr->m_buffer_width, ptr->m_buffer_height);

        auto const it = ptr->m_listeners.find(event_type::framebuffer_resize);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetWindowPosCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t xpos, std::int32_t ypos) {
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
    glfwSetWindowFocusCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t focused) {
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
    // glfwSetWindowIconifyCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t iconified) {
    //     auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
    //     (void)ptr;
    //     (void)iconified;
    // });
    glfwSetWindowMaximizeCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t maximized) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_is_maximized = bool(maximized);
        auto const e = window_maximize_event(0, ptr->m_is_maximized);

        auto const it = ptr->m_listeners.find(event_type::window_maximize);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetWindowContentScaleCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, float xscale, float yscale) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_content_scale_x = double(xscale);
        ptr->m_content_scale_y = double(yscale);
        auto const e = content_scale_event(0, ptr->m_content_scale_x, ptr->m_content_scale_y);

        auto const it = ptr->m_listeners.find(event_type::content_scale);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetCursorPosCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, double xpos, double ypos) {
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
    glfwSetCursorEnterCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t entered) {
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
    glfwSetMouseButtonCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t button, std::int32_t action, std::int32_t mods) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        glfwGetCursorPos(window_ptr, &ptr->m_mouse_x, &ptr->m_mouse_y);
        if (action == GLFW_PRESS) {
            auto const e = mouse_down_event(mouse_button(button), modifier_flags{std::uint32_t(mods)}, ptr->m_mouse_x, ptr->m_mouse_y);
            auto const it = ptr->m_listeners.find(event_type::mouse_down);
            if (it == std::end(ptr->m_listeners)) return;
            auto const& fns = it->second;
            std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                fn.second(e);
            });
        } else {
            auto const e = mouse_up_event(mouse_button(button), modifier_flags{std::uint32_t(mods)}, ptr->m_mouse_x, ptr->m_mouse_y);
            auto const it = ptr->m_listeners.find(event_type::mouse_up);
            if (it == std::end(ptr->m_listeners)) return;
            auto const& fns = it->second;
            std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                fn.second(e);
            });
        }
    });
    glfwSetScrollCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, double xoffset, double yoffset) {
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
    glfwSetKeyCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t key, [[maybe_unused]]std::int32_t scancode, std::int32_t action, std::int32_t mods) {
        [[maybe_unused]]auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));

        if (action != GLFW_RELEASE) {
            auto const e = key_down_event(convert_glfw_keycode(key), convert_glfw_scancode(key), {std::uint32_t(mods)}, action == GLFW_REPEAT);
            auto const it = ptr->m_listeners.find(event_type::key_down);
            if (it == std::end(ptr->m_listeners)) return;
            auto const& fns = it->second;
            std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                fn.second(e);
            });
        } else {
            auto const e = key_up_event(convert_glfw_keycode(key), convert_glfw_scancode(key), {std::uint32_t(mods)});
            auto const it = ptr->m_listeners.find(event_type::key_up);
            if (it == std::end(ptr->m_listeners)) return;
            auto const& fns = it->second;
            std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                fn.second(e);
            });
        }
    });
    glfwSetCharCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::uint32_t codepoint) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        auto const e = key_typed_event(codepoint);
        auto const it = ptr->m_listeners.find(event_type::key_typed);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });
    glfwSetDropCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr, std::int32_t count, char const** paths) {
        auto ptr = static_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        auto const e = drop_event({paths, std::next(paths, count)});
        auto const it = ptr->m_listeners.find(event_type::drop);
        if (it == std::end(ptr->m_listeners)) return;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
    });

    std::int32_t width, height;
    glfwGetWindowSize(static_cast<GLFWwindow*>(m_native), &width, &height);
    m_width  = std::uint32_t(width);
    m_height = std::uint32_t(height);
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(m_native), &width, &height);
    m_buffer_width  = std::uint32_t(width);
    m_buffer_height = std::uint32_t(height);
    float content_scale_x, content_scale_y;
    glfwGetWindowContentScale(static_cast<GLFWwindow*>(m_native), &content_scale_x, &content_scale_y);
    m_content_scale_x = double(content_scale_x);
    m_content_scale_x = double(content_scale_y);
}
auto window::clean_native() -> void {
    glfwDestroyWindow(static_cast<GLFWwindow*>(m_native));
    glfwTerminate();
}
#else
auto window::setup_native() -> void {
    static auto const target_name = "#canvas";
    emscripten_set_window_title(m_title.c_str());
    emscripten_set_canvas_element_size(TARGET_NAME, std::int32_t(m_buffer_width), std::int32_t(m_buffer_height));

    // Create WebGL context
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;
    // attrs.explicitSwapControl = true;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    auto* context = new int();
    *context = emscripten_webgl_create_context(TARGET_NAME, &attrs);
    [[maybe_unused]]auto res = emscripten_webgl_make_context_current(*context);
    // TODO: Check make contet result if it was successful
    m_native = context;

    double width, height;
    emscripten_get_element_css_size(TARGET_NAME, &width, &height);
    auto const device_pixel_ratio = emscripten_get_device_pixel_ratio();
    m_width         = std::uint32_t(width);
    m_height        = std::uint32_t(height);
    m_buffer_width  = std::uint32_t(width  * device_pixel_ratio);
    m_buffer_height = std::uint32_t(height * device_pixel_ratio);
    m_content_scale_x = device_pixel_ratio;
    m_content_scale_y = device_pixel_ratio;
    emscripten_set_canvas_element_size(TARGET_NAME, std::int32_t(m_buffer_width), std::int32_t(m_buffer_height));

    // Callbacks
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE,
    [](int, EmscriptenUiEvent const*, void* userData) {
        auto ptr = static_cast<window*>(userData);
        double width;
        double height;
        emscripten_get_element_css_size(TARGET_NAME, &width, &height);
        if (std::uint32_t(width) != ptr->m_width || std::uint32_t(height) != ptr->m_height) {
            auto const device_pixel_ratio = emscripten_get_device_pixel_ratio();
            ptr->m_width         = std::uint32_t(width);
            ptr->m_height        = std::uint32_t(height);
            ptr->m_buffer_width  = std::uint32_t(width  * device_pixel_ratio);
            ptr->m_buffer_height = std::uint32_t(height * device_pixel_ratio);
            ptr->m_content_scale_x = device_pixel_ratio;
            ptr->m_content_scale_y = device_pixel_ratio;
            emscripten_set_canvas_element_size(TARGET_NAME, std::int32_t(ptr->m_buffer_width), std::int32_t(ptr->m_buffer_height));

            {
                auto const e = window_resize_event(0, ptr->m_width, ptr->m_height);
                auto const it = ptr->m_listeners.find(event_type::window_resize);
                if (it != std::end(ptr->m_listeners)) {
                    auto const& fns = it->second;
                    std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                        fn.second(e);
                    });
                }
            }
            {
                auto const e = framebuffer_resize_event(0, ptr->m_buffer_width, ptr->m_buffer_height);
                auto const it = ptr->m_listeners.find(event_type::framebuffer_resize);
                if (it != std::end(ptr->m_listeners)) {
                    auto const& fns = it->second;
                    std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                        fn.second(e);
                    });
                }
            }
            {
                auto const e = content_scale_event(0, ptr->m_content_scale_x, ptr->m_content_scale_y);
                auto const it = ptr->m_listeners.find(event_type::content_scale);
                if (it != std::end(ptr->m_listeners)) {
                    auto const& fns = it->second;
                    std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
                        fn.second(e);
                    });
                }
            }
        }
        return EM_FALSE;
    });
    emscripten_set_focus_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenFocusEvent const*, void* userData) {
        auto ptr = static_cast<window*>(userData);
        ptr->m_is_focused = true;
        auto const e = window_focus_event(0, ptr->m_is_focused);

        auto const it = ptr->m_listeners.find(event_type::window_focus);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
    });
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenFocusEvent const*, void* userData) {
        auto ptr = static_cast<window*>(userData);
        ptr->m_is_focused = false;
        auto const e = window_focus_event(0, ptr->m_is_focused);

        auto const it = ptr->m_listeners.find(event_type::window_focus);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
    });
    emscripten_set_mousemove_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenMouseEvent const* mouseEvent, void *userData) {
        auto ptr = static_cast<window*>(userData);
        ptr->m_mouse_x = double(mouseEvent->clientX);
        ptr->m_mouse_y = double(mouseEvent->clientY);
        auto const e = mouse_move_event(ptr->m_mouse_x, ptr->m_mouse_y);

        auto const it = ptr->m_listeners.find(event_type::mouse_move);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
    });
    emscripten_set_mousedown_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenMouseEvent const* mouseEvent, void *userData) {
        auto ptr = static_cast<window*>(userData);
        ptr->m_mouse_x = double(mouseEvent->clientX);
        ptr->m_mouse_y = double(mouseEvent->clientY);
        std::uint32_t mod_flag = 0x00;
        mod_flag |= (std::uint32_t(mouseEvent->ctrlKey)  << std::uint32_t(modifier_flags::flag::control));
        mod_flag |= (std::uint32_t(mouseEvent->shiftKey) << std::uint32_t(modifier_flags::flag::shift));
        mod_flag |= (std::uint32_t(mouseEvent->altKey)   << std::uint32_t(modifier_flags::flag::alternative));
        mod_flag |= (std::uint32_t(mouseEvent->metaKey)  << std::uint32_t(modifier_flags::flag::super));

        auto const e = mouse_down_event(mouse_button(mouseEvent->button), {mod_flag}, ptr->m_mouse_x, ptr->m_mouse_y);
        auto const it = ptr->m_listeners.find(event_type::mouse_down);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
    });
    emscripten_set_mouseup_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenMouseEvent const* mouseEvent, void *userData) {
        auto ptr = static_cast<window*>(userData);
        ptr->m_mouse_x = double(mouseEvent->clientX);
        ptr->m_mouse_y = double(mouseEvent->clientY);
        std::uint32_t mod_flag = 0x00;
        mod_flag |= (std::uint32_t(mouseEvent->ctrlKey)  << std::uint32_t(modifier_flags::flag::control));
        mod_flag |= (std::uint32_t(mouseEvent->shiftKey) << std::uint32_t(modifier_flags::flag::shift));
        mod_flag |= (std::uint32_t(mouseEvent->altKey)   << std::uint32_t(modifier_flags::flag::alternative));
        mod_flag |= (std::uint32_t(mouseEvent->metaKey)  << std::uint32_t(modifier_flags::flag::super));

        auto const e = mouse_up_event(mouse_button(mouseEvent->button), {mod_flag}, ptr->m_mouse_x, ptr->m_mouse_y);
        auto const it = ptr->m_listeners.find(event_type::mouse_up);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
    });
    emscripten_set_wheel_callback(target_name, this, EM_FALSE,
    [](int, EmscriptenWheelEvent const* wheelEvent, void *userData) {
        auto ptr = static_cast<window*>(userData);
        auto const e = mouse_wheel_event(wheelEvent->deltaX, wheelEvent->deltaY, ptr->m_mouse_x, ptr->m_mouse_y);
        auto const it = ptr->m_listeners.find(event_type::mouse_wheel);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
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

        // TODO: Handle modifiers
        auto const it = ptr->m_listeners.find(event_type::touch_start);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;

        auto const size = std::size_t(touchEvent->numTouches) <= max_touch_points ? std::size_t(touchEvent->numTouches) : max_touch_points;
        auto const touches = touchEvent->touches;
        touch_points_t points{};
        for (std::size_t i = 0; i < size; ++i) {
            points[i] = touch_point(std::size_t(touches[i].identifier), double(touches[i].clientX), double(touches[i].clientY));
        }

        auto const e = touch_start_event(size, points);
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
    });
    emscripten_set_touchmove_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenTouchEvent const* touchEvent, void* userData) {
        auto ptr = static_cast<window*>(userData);

        auto const it = ptr->m_listeners.find(event_type::touch_move);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;

        auto const size = std::size_t(touchEvent->numTouches) <= max_touch_points ? std::size_t(touchEvent->numTouches) : max_touch_points;
        auto const touches = touchEvent->touches;
        touch_points_t points{};
        for (std::size_t i = 0; i < size; ++i) {
            points[i] = touch_point(std::size_t(touches[i].identifier), double(touches[i].clientX), double(touches[i].clientY));
        }

        auto const e = touch_move_event(size, points);
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
    });
    emscripten_set_touchend_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenTouchEvent const* touchEvent, void* userData) {
        auto ptr = static_cast<window*>(userData);

        auto const it = ptr->m_listeners.find(event_type::touch_end);
        if (it == std::end(ptr->m_listeners)) return EM_FALSE;
        auto const& fns = it->second;

        auto const size = std::size_t(touchEvent->numTouches) <= max_touch_points ? std::size_t(touchEvent->numTouches) : max_touch_points;
        auto const touches = touchEvent->touches;
        touch_points_t points{};
        for (std::size_t i = 0; i < size; ++i) {
            points[i] = touch_point(std::size_t(touches[i].identifier), double(touches[i].clientX), double(touches[i].clientY));
        }

        auto const e = touch_end_event(size, points);
        std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
            fn.second(e);
        });
        return EM_TRUE;
    });
    emscripten_set_touchcancel_callback(target_name, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenTouchEvent const* touchEvent, void* userData) {
        [[maybe_unused]]auto ptr = static_cast<window*>(userData);
        return EM_FALSE;
    });
}
auto window::clean_native() -> void {
    delete static_cast<int*>(m_native);
}
#endif  // __EMSCRIPTEN__

auto loop(window_ref_t window, loop_t fn) -> void {
    static auto _window = window;
    static auto _fn     = fn;

#ifndef __EMSCRIPTEN__
    while (!_window->should_close()) _fn();
#else
    emscripten_set_main_loop([] {
        _fn();
        if (_window->should_close()) emscripten_cancel_main_loop();
    }, -1, EM_TRUE);
#endif  // __EMSCRIPTEN__
}

auto loop(window_ref_t window, loop_dt_t fn) -> void {
    static auto _window = window;
    static auto _fn     = fn;
    static auto previous_time = window->stopwatch();

    // TODO: Fix timestep
#ifndef __EMSCRIPTEN__
    while (!_window->should_close()) {
        auto const now = window->stopwatch();
        auto const delta = now - previous_time;
        previous_time = now;
        _fn(delta);
    }
#else
    emscripten_set_main_loop([] {
        auto const now = _window->stopwatch();
        auto const delta = now - previous_time;
        previous_time = now;
        _fn(delta);
        if (_window->should_close()) emscripten_cancel_main_loop();
    }, -1, EM_TRUE);
#endif  // __EMSCRIPTEN__
}
} // namespace txt
