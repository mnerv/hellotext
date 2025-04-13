#include "window.hpp"
#include <stdexcept>
#include <fstream>
#include <algorithm>

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#define GL_GLEXT_PROTOTYPES 1
#define GL3_PROTOTYPES 1
#define EGL_EGLEXT_PROTOTYPES 1
#include "GL/gl.h"
#else
#include "glad/glad.h"
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

auto graphics_info() -> std::string {
    return fmt::format(R"(Graphics API Info:
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
    if (m_is_init) return;
    setup_native();
    m_is_init = true;

    auto const it = m_listeners.find(event_type::setup);
    if (it == std::end(m_listeners)) return;
    auto const& fns = it->second;
    auto const e = setup_event();
    std::for_each(std::begin(fns), std::end(fns), [&](auto const& fn) {
        fn.second(e);
    });
}

auto window::fullscreen() -> void {
    fullscreen_native();
}

auto window::exit_fullscreen() -> void {
    exit_fullscreen_native();
}

auto window::toggle_fullscreen() -> void {
    if (m_is_fullscreen) exit_fullscreen();
    else fullscreen();
}

auto window::width() const noexcept -> u32 { return m_width; }
auto window::height() const noexcept -> u32 { return m_height; }
auto window::buffer_width() const noexcept -> u32 { return m_buffer_width; }
auto window::buffer_height() const noexcept -> u32 { return m_buffer_height; }
auto window::should_close() const noexcept -> bool { return m_should_close; }
auto window::x() const noexcept -> f64 { return m_position_x; }
auto window::y() const noexcept -> f64 { return m_position_y; }
auto window::content_scale_x() const noexcept -> f64 { return m_content_scale_x; }
auto window::content_scale_y() const noexcept -> f64 { return m_content_scale_y; }
auto window::is_focused() const noexcept -> bool { return m_is_focused; }
auto window::is_hovered() const noexcept -> bool { return m_is_hovered; }
auto window::is_fullscreen() const noexcept -> bool { return m_is_fullscreen; }
auto window::is_init() const noexcept -> bool { return m_is_init; }

auto window::time() const -> f64 {
    auto const t = std::chrono::system_clock::now();
    auto const s = std::chrono::duration<f64>(t.time_since_epoch());
    return s.count();
}
auto window::stopwatch() const -> f64 {
    auto const t = std::chrono::high_resolution_clock::now();
    auto const s = std::chrono::duration<f64>(t.time_since_epoch());
    return s.count();
}

auto window::close() -> void {
    m_should_close = true;
}
auto window::poll() -> void {
    poll_native();
}
auto window::swap() -> void {
    swap_native();
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
    if (fns != std::end(m_listeners))
        fns->second.erase(id);
}

auto loop(window_ref_t ctx, loop_fn_t fn) -> void {
    static auto _win = ctx;
    static auto _fn  = fn;
#ifndef __EMSCRIPTEN__
    while (!_win->should_close()) {
        if (_win->is_init()) _fn();
        _win->poll();
    }
#else
    emscripten_set_main_loop([] {
        if (_win->should_close()) emscripten_cancel_main_loop();
        if (_win->is_init()) _fn();
        _win->poll();
    }, -1, EM_TRUE);
#endif  // __EMSCRIPTEN__
}
} // namespace txt
