#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <filesystem>
#include <memory>
#include <numeric>

#include "fmt/format.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "ft2build.h"
#include FT_FREETYPE_H

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

class window {
public:
    window(std::string const& title, std::int32_t const& width, std::int32_t const& height) {
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error(fmt::format("Failed to initialize GLFW\n"));
    setup_opengl();

    m_native = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (m_native == nullptr) {
        throw std::runtime_error(fmt::format("Failed to create GLFW window\n"));
    }

    glfwMakeContextCurrent(m_native);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        throw std::runtime_error(fmt::format("Failed to initialize GLAD\n"));
    }
    info_opengl();
    }
    ~window() {
        glfwDestroyWindow(m_native);
        glfwTerminate();
    }

    auto native() -> GLFWwindow* { return m_native; }
    auto swap() -> void { glfwSwapBuffers(m_native); }
    auto poll() -> void { glfwPollEvents(); }

private:
    GLFWwindow* m_native;
};

using window_ref_t = std::shared_ptr<window>;
auto new_window(std::string const& title, std::int32_t const& width, std::int32_t const& height) -> window_ref_t {
    return std::make_shared<window>(title, width, height);
}

class texture {
public:
    texture(void const* data, std::uint32_t width, std::uint32_t height, std::uint32_t channels, std::uint32_t format, std::uint32_t pixel_type)
        : m_id(0)
        , m_width(width)
        , m_height(height)
        , m_channels(channels)
        , m_pixel_type(pixel_type) {
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, pixel_type, data);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    ~texture() {
        glDeleteTextures(1, &m_id);
    }

    auto bind(std::size_t index = 0) const -> void {
        glActiveTexture(GLenum(GL_TEXTURE0 + index));
        glBindTexture(GL_TEXTURE_2D, m_id);
    }

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
auto new_texture(void const* data, std::uint32_t width, std::uint32_t height, std::uint32_t channels, std::uint32_t format, std::uint32_t pixel_type) -> texture_ref_t {
    return std::make_shared<texture>(data, width, height, channels, format, pixel_type);
}

struct attrib {
    std::uint32_t count;
    std::string   name;
    std::size_t   index   = 0;
    std::uint32_t divisor = 0;
    std::size_t   offset  = 0;
    std::uint32_t type    = GL_FLOAT;
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
        std::size_t index  = 0;
        std::size_t offset = 0;
        for (auto& a : m_attribs) {
            if (a.index == 0) a.index = index++;
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
    auto bind() -> void {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    }
    auto unbind() -> void {
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
auto new_index_buffer(void const* data, std::size_t bytes, std::size_t size, GLenum type, GLenum usage = GL_STATIC_DRAW) -> index_buffer_ref_t {
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
            glEnableVertexAttribArray(GLuint(a.index));
            glVertexAttribPointer(GLuint(a.index), GLint(a.count), GLenum(a.type), GLboolean(a.normalised), GLsizei(stride), (void const*)a.offset);
            glVertexAttribDivisor(GLuint(a.index), GLuint(a.divisor));
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
            fmt::print("SHADER::{}SHADER::COMPILE_FAILED {}",
                        type == GL_VERTEX_SHADER ? "VERTEX_" : "FRAGMENT_",
                        info_log);
            throw std::runtime_error("Shader compilation error");
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
            fmt::print("SHADER::LINK {}", info_log);
            throw std::runtime_error("Shader linking error");
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

[[maybe_unused]]static constexpr auto vertex_shader = R"glsl(#version 410 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec2 a_uv;

// Instancing
layout(location = 2) in vec2 a_size;
layout(location = 3) in vec2 a_offset;
layout(location = 4) in vec2 a_uv_offset;

out vec2 _uv;
out vec2 _size;
out vec2 _offset;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    _uv = a_uv;
    _size = a_size;
    _offset = a_uv_offset;
    vec3 position = a_position + 0.5;
    position.x *= _size.x;
    position.y *= _size.y;
    position.xy += a_offset - 0.5;
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);
}
)glsl";

[[maybe_unused]]static constexpr auto fragment_shader = R"glsl(#version 410 core
layout(location = 0) out vec4 color;

in vec2 _uv;
in vec2 _size;
in vec2 _offset;

uniform vec2 u_size;
uniform vec3 u_color;
uniform sampler2D u_texture;

void main() {
    vec2 uv = vec2(
        _uv.x * (_size.x / u_size.x) + (_offset.x / u_size.x),
        _uv.y * (_size.y / u_size.y) + (_offset.y / u_size.y)
    );

    float d = texture(u_texture, uv).r;
    float aaf = fwidth(d);
    float alpha = smoothstep(0.5 - aaf, 0.5 + aaf, d);
    color = vec4(u_color, alpha);
}
)glsl";
}

struct character {
    glm::ivec2    size;        // Size of the Glyph
    glm::ivec2    bearing;     // Offset from baseline to left/top of glyph
    std::int64_t  advance;     // Offset to advance to next glyph
    glm::vec2     uv;
    std::vector<std::uint8_t> data;
};

struct gpu_character {
    glm::vec2 size;
    glm::vec2 offset;
    glm::vec2 uv;
};

static constexpr float quad_vertices[] = {
//     x,      y,     z,       u,    v,
    -0.5f,  0.5f,  0.0f,    0.0f, 1.0f,
     0.5f,  0.5f,  0.0f,    1.0f, 1.0f,
     0.5f, -0.5f,  0.0f,    1.0f, 0.0f,
    -0.5f, -0.5f,  0.0f,    0.0f, 0.0f,
};
static constexpr std::uint32_t quad_cw_indices[] = {
    0, 1, 2,
    0, 2, 3
};
static constexpr auto quad_stride     = 5 * sizeof(float);
static constexpr auto quad_xyz_offset = 0 * sizeof(float);
static constexpr auto quad_uv_offset  = 3 * sizeof(float);

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    std::string  title  = "Hello, Text!";
    std::int32_t width  = 738;
    std::int32_t height = 480;
    constexpr auto FONT_SIZE = 32;  // In Pixels
    constexpr auto FT_RENDER_FLAGS = FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    auto window = txt::new_window(title, width, height);

    std::unordered_map<std::uint32_t, character> chars_map;
    bool is_regen_atlas = true;
    glm::ivec2 ch_uv{0.0f, 0.0f};

    FT_Library font_library;
    if (FT_Init_FreeType(&font_library)) {
        fmt::print("Failed initialising FreeType");
        return 1;
    }

    std::string font_path{"deps/fonts/Cozette/CozetteVector.ttf"};
    if (!std::filesystem::exists(font_path)) {
        fmt::print("Font file: {:s} does not exist!", font_path);
        return 1;
    }
    FT_Face font_face;
    auto font_error = FT_New_Face(font_library, font_path.c_str(), 0, &font_face);
    if (font_error == FT_Err_Unknown_File_Format) {
        fmt::print("Unknown file format!");
        return 1;
    } else if (font_error) {
        fmt::print("Failed Loading font!");
        return 1;
    }

    FT_Set_Pixel_Sizes(font_face, 0, FONT_SIZE);
    std::uint32_t max_size = 0;

    auto load_font = [&](FT_Face face, std::uint32_t code) {
        auto const index = FT_Get_Char_Index(face, code);
        if (index == 0) return;
        if (FT_Load_Glyph(face, index, FT_RENDER_FLAGS))
            return;

        auto const width   = face->glyph->bitmap.width;
        auto const height  = face->glyph->bitmap.rows;
        auto const bitmap  = face->glyph->bitmap.buffer;
        auto const left    = face->glyph->bitmap_left;
        auto const top     = face->glyph->bitmap_top;
        auto const advance = face->glyph->advance.x;
        auto const size    = width * height;

        std::vector<std::uint8_t> data;
        data.resize(size);
        std::copy(bitmap, bitmap + size, std::begin(data));

        auto const ch = character{
            {width, height},  // size
            {left, top},      // bearing
            advance,
            {0.0f, 0.0f},     // uv location initialise with 0,0
            data
        };

        auto const& it = chars_map.find(code);
        if (it == std::end(chars_map))
            chars_map.insert({code, std::move(ch)});
        else
            chars_map[code] = std::move(ch);

        auto const width_or_height = std::max(width, height);
        max_size = std::max(max_size, width_or_height);
    };

    // Initial glyph loaded
    for (std::uint32_t code = 0; code < 128; ++code) load_font(font_face, code);

    // Create shader
    auto shader = txt::new_shader(txt::vertex_shader, txt::fragment_shader);
    // Create quad vertex data
    auto quad_vertex  = txt::new_vertex_buffer(quad_vertices, sizeof(quad_vertices), {
        {3, "a_position"},
        {2, "a_uv"}
    });
    // Create character vertex buffer
    auto char_vertex  = txt::new_vertex_buffer(nullptr, sizeof(gpu_character), {
        {2, "a_size",      2, 1},
        {2, "a_offset",    3, 1},
        {2, "a_uv_offset", 4, 1},
    }, GL_DYNAMIC_DRAW);
    auto quad_index   = txt::new_index_buffer(quad_cw_indices, sizeof(quad_cw_indices), txt::len(quad_cw_indices), GL_UNSIGNED_INT);
    auto vertex_array = txt::new_vertex_array();
    vertex_array->add(quad_vertex);
    vertex_array->add(char_vertex);

    txt::texture_ref_t texture{nullptr};

    auto const round_up2 = [](auto const& value) {
        return std::pow(2, std::ceil(std::log2(value) / std::log2(2)));
    };
    auto generate_atlas = [&] {
        auto const cols = static_cast<std::uint32_t>(std::ceil(std::sqrt(chars_map.size())));
        auto const size = static_cast<std::uint32_t>(round_up2(cols * max_size));
        std::vector<std::uint8_t> buffer{};
        buffer.resize(size * size);
        if (is_regen_atlas) ch_uv = {0, 0};

        for (auto& [code, font] : chars_map) {
            auto const width   = font.size.x;
            auto const height  = font.size.y;
            auto const& data = font.data;
            for (std::size_t i = 0; i < height; ++i) {
                for (std::size_t j = 0; j < width; ++j) {
                    auto const data_index = i * width + j;
                    auto const buffer_index = (ch_uv.y + i) * size + (ch_uv.x + j);
                    buffer[buffer_index] = data[data_index];
                }
            }
            // Update character map uv coordinate
            font.uv = {
                ch_uv.x,
                size - ch_uv.y - height // We need to flip this because OpenGL texture is upside down
            };

            ch_uv.x += max_size;
            if (ch_uv.x + max_size >= std::int32_t(size)) {
                ch_uv.x = 0;
                ch_uv.y += max_size;
            }
        }

        // Flip horizontal axis
        for (std::uint32_t i = 0; i < size / 2; i++) {
            for (std::uint32_t j = 0; j < size; j++) {
                auto const top_idx = i * size + j;
                auto const bot_idx = (size - 1 - i) * size + j;
                auto const a = buffer[top_idx];
                auto const b = buffer[bot_idx];
                buffer[top_idx] = b;
                buffer[bot_idx] = a;
            }
        }
        // stbi_write_png("test.png", size, size, 1, buffer.data(), size * 1);

        is_regen_atlas = false;
        texture = txt::new_texture(buffer.data(), size, size, 1, GL_RED, GL_UNSIGNED_BYTE);
        texture->bind();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    };
    generate_atlas();

    std::size_t text_size = 0;
    std::vector<gpu_character> gpu_chars{};
    auto begin_text = [&] {
        text_size = 0;
    };
    auto push_char = [&] (character const& ch, glm::vec2 position) {
        auto const xpos = position.x + float(ch.bearing.x);
        auto const ypos = position.y - float(ch.size.y - ch.bearing.y);
        auto const w = float(ch.size.x);
        auto const h = float(ch.size.y);

        gpu_character gpu_ch{
            {w, h},
            {xpos, ypos},
            ch.uv
        };
        if (text_size < gpu_chars.size())
            gpu_chars[text_size] = gpu_ch;
        else
            gpu_chars.push_back(gpu_ch);
        ++text_size;

    };
    auto push_code = [&] (std::uint32_t code, glm::vec2 position = {0.0f, 0.0f}) {
        auto const& it = chars_map.find(code);
        auto const& ch = it->second;
        push_char(ch, position);
    };
    auto end_text = [&] {
        char_vertex->resize(text_size * sizeof(gpu_character));
        char_vertex->sub_data(gpu_chars.data(), text_size * sizeof(gpu_character));
        vertex_array->bind();
        quad_index->bind();
        glDrawElementsInstanced(GL_TRIANGLES, GLsizei(quad_index->size()), GLenum(quad_index->type()), nullptr, GLsizei(text_size));
    };

    auto render_text = [&](std::string const& text, float const& line_spacing = 1.2f) {
        begin_text();
        std::u32string u32txt;
        utf8::utf8to32(std::begin(text), std::end(text), std::back_inserter(u32txt));
        glm::vec2 pos{};
        for (auto const& code : u32txt) {
            auto it = chars_map.find(code);
            if (it == std::end(chars_map)) {
                load_font(font_face, code);
                it = chars_map.find(code);
                if (it != std::end(chars_map)) {
                    is_regen_atlas = true;
                    generate_atlas();
                } else {
                    it = chars_map.find(0);
                }
            }
            auto const& ch = it->second;

            if (code == '\n') {
                pos.x = 0.0f;
                pos.y -= float(ch.bearing.y) * line_spacing;
            } else {
                push_char(ch, pos);
                pos.x += float(ch.advance >> 6);
            }
        }
        end_text();
    };

    glm::vec3 camera_position{0.0f, 0.0f, 10.0f};
    glm::vec3 camera_front{0.0f, 0.0f, -1.0f};
    glm::vec3 camera_up{0.0f, 1.0f, 0.0f};

    auto is_running = true;
    while(is_running) {
        is_running = !glfwWindowShouldClose(window->native());
        glfwGetFramebufferSize(window->native(), &width, &height);
        if (glfwGetKey(window->native(), GLFW_KEY_Q) == GLFW_PRESS) {
            is_running = false;
        }
        if (glfwGetMouseButton(window->native(), GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        }

        glm::mat4 model{1.0f};
        glm::mat4 view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);
        glm::mat4 projection = glm::ortho(0.0f, float(width), 0.0f, float(height), 0.1f, 100.0f);

        // Clear screen
        glViewport(0, 0, width, height);
        // glEnable(GL_CULL_FACE);
        // glFrontFace(GL_CW);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // render_text(shader_id, "Hello, World!", {25.0f, 25.0f}, 1.0f, {0.5, 0.8f, 0.2f});

        model = glm::translate(model, glm::vec3{200.0f, 200.0f, 0.0f});
        // model = glm::scale(model, glm::vec3(1.0f));
        texture->bind();
        shader->bind();
        shader->set_num("u_texture", 0);
        shader->set_vec2("u_size", {float(texture->width()), float(texture->height())});
        shader->set_vec3("u_color", {0.25f, 0.75f, 1.0f});
        shader->set_mat4("u_model", model);
        shader->set_mat4("u_view", view);
        shader->set_mat4("u_projection", projection);

        render_text("Hello, World!\nHej Charlie! \uf126");

        window->swap();
        window->poll();
    }

    // Clean up FreeType resources
    FT_Done_Face(font_face);
    FT_Done_FreeType(font_library);
    return 0;
}
