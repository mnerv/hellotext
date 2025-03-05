#include "window.hpp"
#include <stdexcept>
#include <fstream>
#include <algorithm>

#include "fmt/format.h"
#include "utf8.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad/glad.h"

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

[[maybe_unused]]static auto info_opengl() -> void {
    fmt::print(R"(Graphics API Info:
Vendor:   {:s}
Renderer: {:s}
Version:  {:s}
Shader:   {:s}
)",
        reinterpret_cast<char const*>(glGetString(GL_VENDOR)),
        reinterpret_cast<char const*>(glGetString(GL_RENDERER)),
        reinterpret_cast<char const*>(glGetString(GL_VERSION)),
        reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION))
    );
}

window::window(window::props const& properties) noexcept
    : m_title(properties.title)
    , m_width(properties.width)
    , m_height(properties.height)
    , m_buffer_width(properties.width)
    , m_buffer_height(properties.height) {
}
window::~window() noexcept {
    clean_native();
}

auto window::setup() -> void {
    setup_native();
    info_opengl();

    auto const it = m_listeners.find(event_type::setup);
    if (it == std::end(m_listeners)) return;
    auto const& fns = it->second;
    auto const e = setup_event();
    std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
        fn.second(e);
    });

    m_is_init = true;
}
auto window::fullscreen() -> void {
    int monitor_count = 0;
    GLFWmonitor** const monitors = glfwGetMonitors(&monitor_count);
    const GLFWvidmode* mode = glfwGetVideoMode(*monitors);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
}

auto window::width() const noexcept -> std::uint32_t { return m_width; }
auto window::height() const noexcept -> std::uint32_t { return m_height; }
auto window::buffer_width() const noexcept -> std::uint32_t { return m_buffer_width; }
auto window::buffer_height() const noexcept -> std::uint32_t { return m_buffer_height; }
auto window::should_close() const noexcept -> bool { return m_should_close; }
auto window::x() const noexcept -> double { return m_position_x; }
auto window::y() const noexcept -> double { return m_position_y; }
auto window::content_scale_x() const noexcept -> double { return m_content_scale_x; }
auto window::content_scale_y() const noexcept -> double { return m_content_scale_y; }
auto window::is_focused() const noexcept -> bool { return m_is_focused; }
auto window::is_hovered() const noexcept -> bool { return m_is_hovered; }
auto window::is_maximized() const noexcept -> bool { return m_is_maximized; }
auto window::is_init() const noexcept -> bool { return m_is_init; }

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
    glfwPollEvents();
}
auto window::swap() -> void {
    glfwSwapBuffers(static_cast<GLFWwindow*>(m_native));
}

auto window::add_event_listener(event_type const& type, std::size_t const& id, event_fn const& fn) -> void {
    if (m_listeners.find(type) == std::end(m_listeners)) {
        event_map fn_map{{id, fn}};
        m_listeners.insert({type, fn_map});
    } else {
        m_listeners[type].insert({id, fn});
    }
}

auto window::remove_event_listener(event_type const& type, std::size_t const& id) -> void {
    auto fns = m_listeners.find(type);
    if (fns != std::end(m_listeners)) {
        fns->second.erase(id);
    }
}

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
    if (value >= 290 && value <= 301 ) {
        scan = txt::scancode(std::uint16_t(txt::scancode::f1) + (value - 290));
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
} // namespace txt
