#include "window.hpp"
#include <stdexcept>
#include <fstream>
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

auto window::make(window::props const& props) -> window::ref_t {
    return std::make_shared<window>(props);
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

window::window(window::props const& props)
    : m_title(props.title)
    , m_width(props.width)
    , m_height(props.height)
    , m_buffer_width(props.width)
    , m_buffer_height(props.height) {
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

#ifndef __EMSCRIPTEN__
static auto setup_opengl() -> void {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

auto window::setup_native() -> void {
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error(fmt::format("Failed to initialize GLFW\n"));
    setup_opengl();

    m_native = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (m_native == nullptr)
        throw std::runtime_error(fmt::format("Failed to create GLFW window\n"));
    glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_native));

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        throw std::runtime_error(fmt::format("Failed to initialize GLAD\n"));

    glfwSetWindowUserPointer(static_cast<GLFWwindow*>(m_native), this);
    glfwSetWindowCloseCallback(static_cast<GLFWwindow*>(m_native), [](GLFWwindow* window_ptr) {
        auto ptr = reinterpret_cast<window*>(glfwGetWindowUserPointer(window_ptr));
        ptr->m_should_close = true;
    });
}
auto window::clean_native() -> void {
    glfwDestroyWindow(static_cast<GLFWwindow*>(m_native));
    glfwTerminate();
}
#else
auto window::setup_native() -> void {
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

    // Callbacks
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_FALSE,
    [](int, [[maybe_unused]]EmscriptenUiEvent const* uiEvent, void* userData) {
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
            emscripten_set_canvas_element_size(TARGET_NAME, std::int32_t(ptr->m_buffer_width), std::int32_t(ptr->m_buffer_height));
        }
        return EM_FALSE;
    });
}
auto window::clean_native() -> void {
    delete static_cast<int*>(m_native);
}
#endif  // __EMSCRIPTEN__

auto loop(window::ref_t window, loop_t fn) -> void {
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
} // namespace txt
