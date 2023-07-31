#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <memory>
#include <numeric>
#include <fstream>
#include <functional>

#include "fmt/format.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_BITMAP_H

#include "glm/vec2.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "stb_image.h"
#include "stb_image_write.h"
#include "utf8cpp/utf8.h"

namespace txt {
template <typename T, std::size_t N>
constexpr auto len(T (&)[N]) -> std::size_t {
    return N;
}

constexpr decltype(auto) curry(auto f, auto... ps){
    if constexpr (requires { std::invoke(f, ps...); }) {
        return std::invoke(f, ps...);
    } else {
        return [f, ps...](auto... qs) -> decltype(auto) { return curry(f, ps..., qs...); };
    }
}

constexpr auto hex2rgba(std::uint32_t color, float alpha = 1.0f) -> glm::vec4 {
    return {
        float((color >> 16 & 0xFF)) / 255.0f,
        float((color >>  8 & 0xFF)) / 255.0f,
        float((color >>  0 & 0xFF)) / 255.0f,
        alpha
    };
}

[[maybe_unused]]static auto setup_opengl() -> void {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

[[maybe_unused]]static auto info_opengl() -> void {
    fmt::print("OpenGL Info:\n");
    fmt::print("    Vendor:   {:s}\n", reinterpret_cast<char const*>(glGetString(GL_VENDOR)));
    fmt::print("    Renderer: {:s}\n", reinterpret_cast<char const*>(glGetString(GL_RENDERER)));
    fmt::print("    Version:  {:s}\n", reinterpret_cast<char const*>(glGetString(GL_VERSION)));
    fmt::print("    Shader:   {:s}\n", reinterpret_cast<char const*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
}

[[maybe_unused]]static auto clear_color(std::uint32_t color, float alpha = 1.0f) -> void {
    auto const r = float((color >> 16) & 0xFF) / 255.0f;
    auto const g = float((color >>  8) & 0xFF) / 255.0f;
    auto const b = float((color >>  0) & 0xFF) / 255.0f;
    glClearColor(r, g, b, alpha);
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

struct window_props {
    std::string  title;
    std::int32_t width;
    std::int32_t height;
};

class window {
public:
    window(window_props const& props) {
        if (glfwInit() == GLFW_FALSE)
            throw std::runtime_error(fmt::format("Failed to initialize GLFW\n"));
        setup_opengl();

        m_native = glfwCreateWindow(props.width, props.height, props.title.c_str(), nullptr, nullptr);
        if (m_native == nullptr) {
            throw std::runtime_error(fmt::format("Failed to create GLFW window\n"));
        }

        glfwMakeContextCurrent(m_native);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            throw std::runtime_error(fmt::format("Failed to initialize GLAD\n"));
        }
        info_opengl();

        // Setup event
        glfwSetWindowUserPointer(m_native, this);
        glfwSetWindowSizeCallback(m_native, [](GLFWwindow* window, int width, int height) {
            auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
            ptr->m_width  = width;
            ptr->m_height = height;
        });
        glfwSetFramebufferSizeCallback(m_native,  [](GLFWwindow* window, int width, int height) {
            auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
            ptr->m_buffer_width  = width;
            ptr->m_buffer_height = height;
        });
        glfwSetWindowCloseCallback(m_native, [](GLFWwindow* window) {
            auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
            ptr->m_should_close = true;
        });
        //glfwSetKeyCallback(m_native, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        //    auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
        //});
        //glfwSetMouseButtonCallback(m_native, [](GLFWwindow* window, int button, int action, int mods) {
        //    auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
        //});
        //glfwSetCursorPosCallback(m_native, [](GLFWwindow* window, double xpos, double ypos) {
        //    auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
        //});
        //glfwSetScrollCallback(m_native, [](GLFWwindow* window, double xoffset, double yoffset) {
        //    auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
        //});
        //glfwSetCharCallback(m_native, [](GLFWwindow* window, std::uint32_t codepoint) {
        //    auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
        //});
        //glfwSetDropCallback(m_native, [](GLFWwindow* window, int count, const char** paths) {
        //    auto ptr = reinterpret_cast<txt::window*>(glfwGetWindowUserPointer(window));
        //});

        glfwGetWindowSize(m_native, &m_width, &m_height);
        glfwGetFramebufferSize(m_native, &m_buffer_width, &m_buffer_height);
    }
    ~window() {
        glfwDestroyWindow(m_native);
        glfwTerminate();
    }

    auto width() const -> std::int32_t { return m_width; }
    auto height() const -> std::int32_t { return m_height; }
    auto buffer_width() const -> std::int32_t { return m_buffer_width; }
    auto buffer_height() const -> std::int32_t { return m_buffer_height; }
    auto should_close() const -> bool { return m_should_close; }
    auto native() -> GLFWwindow* { return m_native; }
    auto time() -> double { return glfwGetTime(); }

    auto close() -> void { m_should_close = true; }
    auto swap() -> void { glfwSwapBuffers(m_native); }
    auto poll() -> void { glfwPollEvents(); }

private:
    GLFWwindow*  m_native;

    std::int32_t m_width{0};
    std::int32_t m_height{0};
    std::int32_t m_buffer_width{0};
    std::int32_t m_buffer_height{0};
    bool         m_should_close{false};
};

using window_ref_t = std::shared_ptr<window>;
auto new_window(window_props const& props) -> window_ref_t {
    return std::make_shared<window>(props);
}

class image {
public:
    image(std::uint8_t const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels)
        : m_buffer(nullptr)
        , m_width(width)
        , m_height(height)
        , m_channels(channels)
        , m_size(width * height * channels) {
        m_buffer = new std::uint8_t[m_size];
        std::copy(data, data + m_size, m_buffer);
    }
    image(std::size_t const& width, std::size_t const& height, std::size_t const& channels)
        : m_buffer(new std::uint8_t[width * height * channels])
        , m_width(width)
        , m_height(height)
        , m_channels(channels)
        , m_size(width * height * channels) {
        std::fill_n(m_buffer, m_size, 0);
    }
    image(image const& img)
        : m_buffer(nullptr)
        , m_width(img.m_width)
        , m_height(img.m_height)
        , m_channels(img.m_channels)
        , m_size(m_width * m_height * m_channels) {
        m_buffer = new std::uint8_t[m_size];
        std::memcpy(m_buffer, img.m_buffer, m_size);
    }
    ~image() { delete[] m_buffer; }

    auto width() const noexcept -> std::size_t { return m_width; }
    auto height() const noexcept -> std::size_t { return m_height; }
    auto channels() const noexcept -> std::size_t { return m_channels; }
    auto data() const noexcept -> std::uint8_t const* { return m_buffer; }
    auto pixel(std::size_t const& x, std::size_t const& y) const noexcept -> glm::vec4 {
        if (x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1) return {};
        auto const index = (y * m_channels * m_width) + (x * m_channels);
        glm::vec4 color{};
        for (std::size_t i = 0; i < m_channels; i++)
            color[std::int32_t(i)] = float(m_buffer[index + i]) / 255.0f;
        return color;
    };

    auto set_pixel(std::size_t const& x, std::size_t const& y, glm::vec4 const& color) const noexcept -> void {
        if (x < 0 || x > m_width - 1 || y < 0 || y > m_height - 1) return;
        auto const index = (y * m_channels * m_width) + (x * m_channels);
        for (std::size_t i = 0; i < m_channels && i < 4; i++)
            m_buffer[index + i] = std::uint8_t(color[std::int32_t(i)] * 255.0f);
    }

    auto fliph() -> void {
        for (std::size_t i = 0; i < m_height / 2; i++) {
            for (std::uint32_t j = 0; j < m_width; j++) {
                auto const a = pixel(j, i);
                auto const b = pixel(j, m_height - i - 1);
                set_pixel(j, i, b);
                set_pixel(j, m_height - i - 1, a);
            }
        }
    }
    auto write_png(std::string const& filename) const -> void {
        stbi_write_png(filename.c_str(), std::int32_t(m_width), std::int32_t(m_height), std::int32_t(m_channels), m_buffer, std::int32_t(m_width * m_channels));
    }

private:
    std::uint8_t* m_buffer;
    std::size_t   m_width;
    std::size_t   m_height;
    std::size_t   m_channels;
    std::size_t   m_size;
};

using image_ref_t = std::shared_ptr<image>;
auto new_image(std::uint8_t const* data, std::size_t const& width, std::size_t const& height, std::size_t const& channels) -> image_ref_t {
    return std::make_shared<image>(data, width, height, channels);
}
auto new_image(std::size_t const& width, std::size_t const& height, std::size_t const& channels) -> image_ref_t {
    return std::make_shared<image>(width, height, channels);
}

class texture {
public:
    texture(void const* data, std::uint32_t width, std::uint32_t height, std::uint32_t channels, std::uint32_t internal_format, std::uint32_t format, std::uint32_t pixel_type)
        : m_id(0)
        , m_width(width)
        , m_height(height)
        , m_channels(channels)
        , m_pixel_type(pixel_type) {
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_width, m_height, 0, format, pixel_type, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    ~texture() {
        glDeleteTextures(1, &m_id);
    }

    auto bind(std::size_t index = 0) const -> void {
        glActiveTexture(GLenum(GL_TEXTURE0 + index));
        glBindTexture(GL_TEXTURE_2D, m_id);
    }
    auto unbind() const -> void {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    auto channels() const -> std::uint32_t { return m_channels; }
    auto pixel_type() const -> std::uint32_t { return m_pixel_type; }
    auto width() const -> std::uint32_t { return m_width; }
    auto height() const -> std::uint32_t { return m_height; }

private:
    std::uint32_t m_id;
    std::uint32_t m_width;
    std::uint32_t m_height;
    std::uint32_t m_channels;
    std::uint32_t m_pixel_type;
};

using texture_ref_t = std::shared_ptr<texture>;
auto new_texture(void const* data, std::uint32_t width, std::uint32_t height, std::uint32_t channels, std::uint32_t internal_format, std::uint32_t format, std::uint32_t pixel_type) -> texture_ref_t {
    return std::make_shared<texture>(data, width, height, channels, internal_format, format, pixel_type);
}

struct attrib {
    std::size_t   count;
    std::string   name;
    std::size_t   divisor = 0;
    std::uint32_t type    = GL_FLOAT;
    std::size_t   offset  = 0;
    bool          normalised = false;

    static inline constexpr auto type_size(GLenum type) -> std::size_t {
        switch (type) {
            case GL_FLOAT:
            default: return sizeof(float);
        }
    }
};

class vertex_buffer {
public:
    vertex_buffer(void const* data, std::size_t bytes, std::initializer_list<attrib> const& attribs, GLenum usage)
        : m_id(0)
        , m_bytes(bytes)
        , m_usage(usage)
        , m_attribs(attribs) {
        glGenBuffers(1, &m_id);
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_bytes), data, GLenum(m_usage));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        compute_offset();
    }
    ~vertex_buffer() {
        glDeleteBuffers(1, &m_id);
    }

    auto layout() const -> std::vector<attrib> const& { return m_attribs; }
    auto bind() const -> void {
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
    }
    auto unbind() const -> void {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    auto sub_data(void const* data, std::size_t bytes, std::size_t offset = 0) -> void {
        assert(data != nullptr || (bytes + offset) > m_bytes);
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
        glBufferSubData(GL_ARRAY_BUFFER, offset, bytes, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    auto resize(std::size_t bytes) -> void {
        if (m_bytes == bytes || bytes == 0) return;
        m_bytes = bytes;
        glBindBuffer(GL_ARRAY_BUFFER, m_id);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_bytes), nullptr, GLenum(m_usage));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

private:
    auto compute_offset() -> void {
        std::size_t offset = 0;
        for (auto& a : m_attribs) {
            a.offset = offset;
            offset += a.count * attrib::type_size(a.type);
        }
    }

private:
    std::uint32_t m_id;
    std::size_t   m_bytes;
    std::uint32_t m_usage;
    std::vector<attrib> m_attribs;
};

using vertex_buffer_ref_t = std::shared_ptr<vertex_buffer>;
auto new_vertex_buffer(void const* data, std::size_t bytes, std::initializer_list<attrib> const& attribs, GLenum usage = GL_STATIC_DRAW) -> vertex_buffer_ref_t {
    return std::make_shared<vertex_buffer>(data, bytes, attribs, usage);
}

class index_buffer {
public:
    index_buffer(void const* data, std::size_t bytes, std::size_t size, GLenum type, GLenum usage)
        : m_id(0)
        , m_bytes(bytes)
        , m_size(size)
        , m_type(type)
        , m_usage(usage) {
        glGenBuffers(1, &m_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_bytes), data, GLenum(m_usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    ~index_buffer() {
        glDeleteBuffers(1, &m_id);
    }

    auto size() const -> std::size_t { return m_size; }
    auto type() const -> std::uint32_t { return m_type; }
    auto bind() const -> void {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    }
    auto unbind() const -> void {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    auto sub_data(void const* data, std::size_t bytes, std::size_t offset = 0) -> void {
        assert(data != nullptr && (bytes + offset) < m_bytes);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, bytes, data);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    auto resize(std::size_t bytes) -> void {
        if (m_bytes == bytes || bytes == 0) return;
        m_bytes = bytes;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(m_bytes), nullptr, GLenum(m_usage));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

private:
    std::uint32_t m_id;
    std::size_t   m_bytes;
    std::size_t   m_size;
    std::uint32_t m_type;
    std::uint32_t m_usage;
};

using index_buffer_ref_t = std::shared_ptr<index_buffer>;
auto new_index_buffer(void const* data, std::size_t bytes, std::size_t size, GLenum type = GL_UNSIGNED_INT, GLenum usage = GL_STATIC_DRAW) -> index_buffer_ref_t {
    return std::make_shared<index_buffer>(data, bytes, size, type, usage);
}

class vertex_array {
public:
    vertex_array()
        : m_id(0) {
        glGenVertexArrays(1, &m_id);
    }
    ~vertex_array() {
        glDeleteVertexArrays(1, &m_id);
    }

    auto add(vertex_buffer_ref_t buffer) -> void {
        glBindVertexArray(m_id);
        buffer->bind();
        auto const& layout = buffer->layout();
        auto const stride = compute_stride(layout);
        std::for_each(std::begin(layout), std::end(layout), [&](attrib const& a) {
            if (a.count <= 4) {
                glEnableVertexAttribArray(GLuint(m_index));
                glVertexAttribPointer(GLuint(m_index), GLint(a.count), GLenum(a.type), GLboolean(a.normalised), GLsizei(stride), (void const*)a.offset);
                glVertexAttribDivisor(GLuint(m_index++), GLuint(a.divisor));
            } else if (a.count == 4 * 4) {
                for (std::size_t i = 0; i < 4; ++i) {
                    glEnableVertexAttribArray(GLuint(m_index));
                    glVertexAttribPointer(GLuint(m_index), 4, GLenum(a.type), GLboolean(a.normalised), GLsizei(stride), (void const*)(a.offset + i * sizeof(glm::vec4)));
                    glVertexAttribDivisor(GLuint(m_index++), GLuint(a.divisor));
                }
            }
        });
        m_buffers.push_back(std::move(buffer));
        glBindVertexArray(0);
    }
    auto bind() const -> void {
        glBindVertexArray(m_id);
        for (auto const& buffer : m_buffers)
            buffer->bind();
    }
    auto unbind() const -> void {
        glBindVertexArray(0);
    }

private:
    auto compute_stride(std::vector<attrib> const& attribs) const -> std::size_t {
        return std::accumulate(std::begin(attribs), std::end(attribs), std::size_t(0),
        [](auto const& acc, auto const& b) {
            auto const size = b.count * attrib::type_size(GLenum(b.type));
            return acc + size;
        });
    }

private:
    std::uint32_t m_id;
    std::size_t   m_index{0};
    std::vector<vertex_buffer_ref_t> m_buffers{};
};

using vertex_array_ref_t = std::shared_ptr<vertex_array>;
auto new_vertex_array() -> vertex_array_ref_t {
    return std::make_shared<vertex_array>();
}

class shader {
public:
    shader(std::string const& vs_src, std::string const& fs_src) : m_id(0) {
        auto vs = compile(GL_VERTEX_SHADER, vs_src.c_str());
        auto fs = compile(GL_FRAGMENT_SHADER, fs_src.c_str());
        m_id = link(vs, fs);
    }
    ~shader() {
        glDeleteProgram(m_id);
    }

    auto id() const -> std::uint32_t { return m_id; }
    auto bind() const -> void {
        glUseProgram(m_id);
    }
    auto unbind() const -> void {
        glUseProgram(0);
    }

    auto set_num(std::string const& name, std::uint32_t value) -> void {
        glUniform1ui(location(name), value);
    }

    auto set_vec2(std::string const& name, glm::vec2 const& vec) -> void {
        glUniform2fv(location(name), 1, glm::value_ptr(vec));
    }
    auto set_vec3(std::string const& name, glm::vec3 const& vec) -> void {
        glUniform3fv(location(name), 1, glm::value_ptr(vec));
    }
    auto set_vec4(std::string const& name, glm::vec4 const& vec) -> void {
        glUniform4fv(location(name), 1, glm::value_ptr(vec));
    }

    auto set_mat4(std::string const& name, glm::mat4 const& mat, bool const& transpose = false) -> void {
        glUniformMatrix4fv(location(name), 1, (transpose ? GL_TRUE : GL_FALSE), glm::value_ptr(mat));
    }

private:
    auto location(std::string const& name) -> std::int32_t {
        return glGetUniformLocation(m_id, name.c_str());
    }

    auto compile(std::uint32_t const& type, char const* source) -> std::uint32_t {
        std::uint32_t shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        std::int32_t is_success;
        constexpr auto LOG_SIZE = 512;
        static char info_log[LOG_SIZE];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &is_success);
        if (!is_success) {
            glGetShaderInfoLog(shader, LOG_SIZE, nullptr, info_log);
            auto const err_str = fmt::format("shader compile error [{}] - {}",
                type == GL_VERTEX_SHADER ? "vertex_" : "fragment",
                info_log);
            throw std::runtime_error(err_str);
        }

        return shader;
    }

    auto link(std::uint32_t const& vs, std::uint32_t const& fs) -> std::uint32_t {
        std::uint32_t program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        std::int32_t is_success;
        constexpr auto LOG_SIZE = 512;
        static char info_log[LOG_SIZE];
        glGetProgramiv(program, GL_LINK_STATUS, &is_success);
        if (!is_success) {
            glGetProgramInfoLog(program, LOG_SIZE, nullptr, info_log);
            auto const err_str = fmt::format("shader link error - ", info_log);
            throw std::runtime_error(err_str);
        }

        glUseProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return program;
    }

private:
    std::uint32_t m_id;
};

using shader_ref_t = std::shared_ptr<shader>;
auto new_shader(std::string const& vs_src, std::string const& fs_src) -> shader_ref_t {
    return std::make_shared<shader>(vs_src, fs_src);
}

enum class tf_render_mode {
    normal,
    sdf,
    subpixel,
    raster
};

struct glyph {
    std::uint32_t code;             // UTF-32 code point
    glm::ivec2    bearing;          // Offset from baseline to left/top of glyph
    glm::i64vec2  advance;          // Offset to advance to next glyph
    image_ref_t   bitmap{nullptr};  // Loaded bitmap
    glm::ivec2    uv{0, 0};         // uv offset in texture atlas in pixels
};
using glyph_set_t = std::unordered_map<std::uint32_t, glyph>;

struct font_props {
    std::string    filename;
    std::uint32_t  size;
    tf_render_mode mode{tf_render_mode::normal};
};

struct font_t {
    std::string    filename;
    std::uint32_t  pixel_size;
    texture_ref_t  texture;
    image_ref_t    atlas;
    glyph_set_t    glyphs;
    glm::ivec2     uv;
    std::uint32_t  atlas_size;
    std::uint32_t  max_size;  // max size of a glyph in the font
    tf_render_mode mode;
    bool           is_invalid;
    std::uint32_t  channels;
    std::int32_t   flags;
    FT_Face        face;
};

class font_manager{
public:
    font_manager(font_props const& props) {
        if (FT_Init_FreeType(&m_library))
            throw std::runtime_error("Failed to initialse FreeType library");
        FT_Bitmap_Init(&m_bitmap);

        m_font.filename   = props.filename;
        m_font.pixel_size = props.size;
        m_font.mode       = props.mode;
        m_font.is_invalid = true;

        load();
    }
    ~font_manager() {
        FT_Bitmap_Done(m_library, &m_bitmap);
        FT_Done_FreeType(m_library);
    }

    auto texture() const -> texture_ref_t const& { return m_font.texture; }
    auto at(std::uint32_t codepoint) -> glyph const& {
        auto it = m_font.glyphs.find(codepoint);
        if (it == std::end(m_font.glyphs)) {
            // TODO: Try to load it and rebuild the atlas
            return m_font.glyphs.at('A');
        }
        return it->second;
    }
    auto font() const -> font_t const& { return m_font; }

private:
    auto load() -> void {
        m_font.channels = 1;
        std::int32_t flags = FT_LOAD_RENDER;
        if (m_font.mode == tf_render_mode::raster) {
            // Do nothing, we render as usual
        } else if (m_font.mode == tf_render_mode::sdf) {
            flags |=  FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
        } else if (m_font.mode == tf_render_mode::subpixel) {
            flags |= FT_LOAD_TARGET_(FT_RENDER_MODE_LCD);
            FT_Library_SetLcdFilter(m_library, FT_LCD_FILTER_DEFAULT);
            m_font.channels = 3;
        }
        m_font.flags = flags;

        if (!std::filesystem::exists(m_font.filename)) {
            throw std::runtime_error(fmt::format("Font file path '{}' does not exist.", m_font.filename));
        }

        auto const ff_ec = FT_New_Face(m_library, m_font.filename.c_str(), 0, &m_font.face);
        if (ff_ec == FT_Err_Unknown_File_Format)
            throw std::runtime_error(fmt::format("Font file path '{}' have an unknown file format.", m_font.filename));
        else if(ff_ec)
            throw std::runtime_error("Error loading font file.");

        FT_Set_Pixel_Sizes(m_font.face, 0, m_font.pixel_size);
        // Load inital characters
        for (std::uint32_t code = 0; code < 128; ++code)
            load_typeface(code);

        resize_cache();
        reload_cache();
    }

    auto load_typeface(std::uint32_t code) -> void {
        auto const index = FT_Get_Char_Index(m_font.face, code);
        if (index == 0) return;
        if (FT_Load_Glyph(m_font.face, index, m_font.flags))
            return;

        auto const width     = m_font.face->glyph->bitmap.width / m_font.channels;
        auto const height    = m_font.face->glyph->bitmap.rows;
        auto const left      = m_font.face->glyph->bitmap_left;
        auto const top       = m_font.face->glyph->bitmap_top;
        auto const advance_x = m_font.face->glyph->advance.x;
        auto const advance_y = m_font.face->size->metrics.height;

        // Convert to one byte alignment
        FT_Bitmap_Convert(m_library, &m_font.face->glyph->bitmap, &m_bitmap, 1);
        auto img = new_image(m_bitmap.buffer, width, height, m_font.channels);

        glyph tf{
            .code    = code,
            .bearing = {left,  top},
            .advance = {advance_x, advance_y},
            .bitmap  = img,
            .uv      = {0, 0}
        };
        m_font.glyphs[code] = std::move(tf);

        auto const width_or_height = std::max(width, height);
        m_font.max_size = std::max(m_font.max_size, width_or_height);
    }

    auto reload_all() -> void {
        for (auto const& [code, tf] : m_font.glyphs)
            load_typeface(code);

        resize_cache();
        reload_cache();
    }

    auto reload_cache() -> void {
        for (auto& [code, font] : m_font.glyphs)
            insert_atlas(font);

        std::uint32_t internal_format = GL_R8;
        std::uint32_t format = GL_RED;
        if (m_font.channels == 3) {
            internal_format = GL_RGB8;
            format = GL_RGB;
        }

        m_font.texture = new_texture(m_font.atlas->data(), std::uint32_t(m_font.atlas->width()), std::uint32_t(m_font.atlas->height()), std::uint32_t(m_font.atlas->channels()), internal_format, format, GL_UNSIGNED_BYTE);
        m_font.texture->bind();
        if (m_font.mode == tf_render_mode::raster) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        m_font.texture->unbind();
    }

    auto resize_cache() -> void {
        constexpr auto round_up2 = [](auto const& value) {
            return std::pow(2, std::ceil(std::log2(value) / std::log2(2)));
        };
        auto const cols = static_cast<std::uint32_t>(std::ceil(std::sqrt(m_font.glyphs.size())));
        auto const msp2 = static_cast<std::uint32_t>(round_up2(m_font.max_size));  // Glyph max size round to power of 2
        m_font.atlas_size = static_cast<std::uint32_t>(round_up2(cols * msp2));
        if (m_font.atlas == nullptr || m_font.atlas_size != m_font.atlas->width()) {
            m_font.atlas = txt::new_image(m_font.atlas_size, m_font.atlas_size, m_font.channels);
            m_font.uv = {0, std::int32_t(m_font.atlas_size) - 1};
        }
    }

    auto insert_atlas(glyph& tf) -> void {
        auto const& bm = tf.bitmap;
        for (std::size_t i = 0; i < bm->height(); ++i) {
            for (std::size_t j = 0; j < bm->width(); ++j) {
                auto const pixel = bm->pixel(j, i);
                m_font.atlas->set_pixel(std::size_t(m_font.uv.x) + j, std::size_t(m_font.uv.y) - i, pixel);
            }
        }
        tf.uv = {
            m_font.uv.x,
            m_font.uv.y - std::int32_t(bm->height() - 1)
        };

        m_font.uv.x += m_font.max_size;
        if (m_font.uv.x >= std::int32_t(m_font.atlas->width() - m_font.max_size)) {
            m_font.uv.x = 0;
            m_font.uv.y -= m_font.max_size;
        }
    }

private:
    FT_Library m_library{};
    FT_Bitmap  m_bitmap{};
    font_t     m_font{};
};

using font_manager_ref_t = std::shared_ptr<font_manager>;
auto new_font_manager(font_props const& props) -> font_manager_ref_t {
    return std::make_shared<font_manager>(props);
}

struct transform {
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};
};

class text_renderer {
public:
    struct gpu {
        glm::vec2 size;
        glm::vec2 uv;
        glm::vec4 color;
        glm::mat4 model;
    };

public:
    text_renderer(font_manager_ref_t const& font_manager, std::string const& vertex_shader_path, std::string const& fragment_shader_path)
        : m_manager(font_manager), m_size(0), m_va(new_vertex_array()) {
        static constexpr float quad_vertices[] = {
        //     x,     y,     z,       u,    v,
            0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
            0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
            1.0f,  1.0f,  0.0f,    1.0f, 1.0f,
            1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
        };
        static constexpr std::uint32_t quad_cw_indices[] = {
            0, 1, 2,
            0, 2, 3
        };
        m_va->add(new_vertex_buffer(quad_vertices, sizeof(quad_vertices), {
            {.count = 3, .name = "a_position"},
            {.count = 2, .name = "a_uv"},
        }));
        m_ib = new_index_buffer(quad_cw_indices, sizeof(quad_cw_indices), len(quad_cw_indices));
        m_tb = new_vertex_buffer(nullptr, 0, {
            {.count = 2,     .name = "a_size"  , .divisor = 1},
            {.count = 2,     .name = "a_offset", .divisor = 1},
            {.count = 4,     .name = "a_color" , .divisor = 1},
            {.count = 4 * 4, .name = "a_model" , .divisor = 1},
        }, GL_DYNAMIC_DRAW);
        m_va->add(m_tb);

        auto vs = read_text(vertex_shader_path);
        auto fs = read_text(fragment_shader_path);
        if (m_manager->font().mode == tf_render_mode::sdf) {
            auto const line = fs.find('\n');
            fs.insert(line + 1, "#define RENDER_MODE SDF\n");
        } else if (m_manager->font().mode == tf_render_mode::subpixel) {
            auto const line = fs.find('\n');
            fs.insert(line + 1, "#define RENDER_MODE SUBPIXEL\n");
        }
        m_shader = new_shader(vs, fs);
    }

    auto shader() const -> shader_ref_t const& { return m_shader; }
    auto size() const -> std::size_t { return m_size; }
    auto text_size(std::string const& txt) -> glm::vec2 {
        std::u32string str{};
        utf8::utf8to32(std::begin(txt), std::end(txt), std::back_inserter(str));
        glm::vec2 position{0.0f};
        std::int64_t advance_y = 0;
        for (auto const& code : str) {
            if (code == '\n') {
                position.x = 0.0f;
                position.y += float(advance_y >> 6);
                continue;
            }
            auto const& tf = m_manager->at(code);
            position.x += float(tf.advance.x >> 6);
            advance_y = tf.advance.y;
        }
        position.y += float(advance_y >> 6);
        return position;
    }
    auto begin() -> void {
        m_size = 0;
        glEnable(GL_BLEND);
        if (m_manager->font().mode == tf_render_mode::subpixel) {
            glBlendFunc(GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR);
        } else {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
    auto render(std::string const& txt, glm::vec2 const& position = {}, std::uint32_t color = 0xFFFFFF, float alpha = 1.0f) -> glm::vec2 {
        auto const model = glm::translate(glm::mat4{1.0f}, glm::vec3{position, 0.0f});
        return render(txt, model, color, alpha);
    }
    auto render(std::string const& txt, transform const& tf, std::uint32_t color = 0xFFFFFF, float alpha = 1.0f) -> glm::vec2 {
        auto model = glm::translate(glm::mat4{1.0f}, tf.position);
        auto const rotate_x = glm::rotate(glm::mat4{1.0f}, tf.rotation.x, {1.0f, 0.0f, 0.0f});
        auto const rotate_y = glm::rotate(glm::mat4{1.0f}, tf.rotation.y, {1.0f, 0.0f, 0.0f});
        auto const rotate_z = glm::rotate(glm::mat4{1.0f}, tf.rotation.z, {1.0f, 0.0f, 0.0f});
        model = model * rotate_x * rotate_y * rotate_z;
        model = glm::scale(model, tf.scale);
        return render(txt, model, color, alpha);
    }
    auto render(std::string const& txt, glm::mat4 const& model, std::uint32_t color = 0xFFFFFF, float alpha = 1.0f) -> glm::vec2 {
        auto const color_f32s = hex2rgba(color, alpha);
        std::u32string str{};
        utf8::utf8to32(std::begin(txt), std::end(txt), std::back_inserter(str));
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        std::int64_t advance_y = 0;
        for (auto const& code : str) {
            if (code == '\n') {
                position.x = 0.0f;
                position.y -= float(advance_y >> 6);
                continue;
            }
            auto const& tf = m_manager->at(code);
            push(tf, glm::translate(model, position), color_f32s);
            position.x += float(tf.advance.x >> 6);
            advance_y = tf.advance.y;
        }
        position.y += float(advance_y >> 6);
        return position;
    }
    auto end() -> void {
        if (m_size == 0) return;

        m_shader->bind();
        m_shader->set_vec2("u_size", {float(m_manager->texture()->width()), float(m_manager->texture()->height())});
        m_manager->texture()->bind();
        m_shader->set_num("u_texture", 0);

        m_tb->resize(m_size * sizeof(gpu));
        m_tb->sub_data(m_cache.data(), m_size * sizeof(gpu));
        m_va->bind();
        m_ib->bind();
        glDrawElementsInstanced(GL_TRIANGLES, GLsizei(m_ib->size()), GLenum(m_ib->type()), nullptr, GLsizei(m_size));
        m_shader->unbind();
    }

public:
    auto push(glyph const& tf, glm::mat4 model, glm::vec4 color) -> void {
        auto const xpos = float(tf.bearing.x);
        auto const ypos = -(float(tf.bitmap->height()) - float(tf.bearing.y));
        auto const w = float(tf.bitmap->width());
        auto const h = float(tf.bitmap->height());
        auto const u = float(tf.uv.x);
        auto const v = float(tf.uv.y);
        model = glm::translate(model, {xpos, ypos, 0.0f});

        gpu ch{
            .size  = {w, h},
            .uv    = {u, v},
            .color = color,
            .model = model
        };

        if (m_size < m_cache.size()) {
            m_cache[m_size] = std::move(ch);
        } else {
            m_cache.push_back(std::move(ch));
        }
        ++m_size;
    }

private:
    font_manager_ref_t m_manager;
    std::size_t        m_size;

    vertex_array_ref_t  m_va{};
    index_buffer_ref_t  m_ib{};
    vertex_buffer_ref_t m_tb{};
    shader_ref_t        m_shader{};

    std::vector<gpu> m_cache{};
};
}

auto run() -> void {
    auto window = txt::new_window({
        "Hello, Text!",
        1280, 800
    });

    auto font_manager = txt::new_font_manager({
        .filename = "deps/fonts/Cozette/CozetteVector.ttf",
        .size     = 13,
        .mode     = txt::tf_render_mode::raster
    });
    // auto font_manager = txt::new_font_manager({
    //     .filename = "deps/fonts/SFMono/SFMono Regular Nerd Font Complete.otf",
    //     .size     = 11,
    // });
    txt::text_renderer text{
        font_manager,
        "shaders/text.vert",
        "shaders/text.frag"
    };

    double current_time = window->time();
    double previous_time = window->time();

    std::size_t instances = 0;
    while(!window->should_close()) {
        auto const width  = window->width();
        auto const height = window->height();

        glm::mat4 model{1.0f};
        model = glm::translate(model, {0, float(height) - float(font_manager->font().pixel_size), 0.0f});
        auto const view = glm::lookAt(glm::vec3{0.0, 0.0, 1.0}, glm::vec3{0.0, 0.0, 0.0}, glm::vec3{0.0, 1.0, 0.0});
        auto const projection = glm::ortho(0.0f, float(width), 0.0f, float(height));

        previous_time = current_time;
        current_time = window->time();
        auto const delta_time = current_time - previous_time;

        text.shader()->bind();
        text.shader()->set_mat4("u_model", model);
        text.shader()->set_mat4("u_view", view);
        text.shader()->set_mat4("u_projection", projection);

        glViewport(0, 0, window->buffer_width(), window->buffer_height());
        txt::clear_color(0x141414);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        text.begin();
        auto const fps_text = fmt::format("{:.2f} ms", delta_time * 1000.0);
        auto const fps_text_size = text.render(fps_text, {float(width) - text.text_size(fps_text).x - 5.0f, 0.0f});
        auto txt = "Hello, World!";
        auto text_size = text.text_size(txt);
        text.render(txt, txt::transform{
            {float(width / 2) - text_size.x / 2 * 5, -float(height / 2), 0.0f},
            {0.0f, 0.0f, 0.0f},
            {5.0f, 5.0f, 1.0f}
        }, 0xff899f);

        text.render("Hej, Charlie!", {5.0f, 0.0f}, 0x9789ff);

        txt = "Trying to feel alive";
        text_size = text.text_size(txt);
        text.render(txt, {5.0f, -float(height) + text_size.y + 5.0f}, 0xa0cfd8);

        auto const instance_count_text = fmt::format("{} instances", instances);
        text.render(instance_count_text, {float(width) - text.text_size(instance_count_text).x - 5.0f, -fps_text_size.y});
        instances = text.size();
        text.end();

        window->swap();
        window->poll();
    }
}

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    try {
        run();
    } catch (std::exception const& e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
    return 0;
}
