#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <filesystem>

#include "spdlog/spdlog.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "glm/vec2.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "stb_image.h"
#include "stb_image_write.h"

namespace one {
static auto setup_opengl() -> void {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

[[maybe_unused]]static auto vertex_shader = R"(#version 410 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_uv;

out vec2 io_uv;
uniform mat4 u_projection;

void main() {
    io_uv = a_uv;
    gl_Position = u_projection * vec4(a_position, 0.0f, 1.0f);
    //gl_Position = vec4(a_position, 0.0f, 1.0f);
}
)";

[[maybe_unused]]static auto fragment_shader = R"(#version 410 core
layout(location = 0) out vec4 color;
in vec2 io_uv;

uniform vec3 u_textcolor;
uniform sampler2D u_texture;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_texture, io_uv).r);
    color = vec4(u_textcolor, 1.0) * sampled;
}
)";

struct character {
    std::uint32_t texture_id;  // Texture ID of the Glyph
    glm::ivec2    size;        // Size of the Glyph
    glm::ivec2    bearing;     // Offset from baseline to left/top of glyph
    std::int64_t  advance;     // Offset to advance to next glyph
};

auto uniform_name(std::uint32_t const& shader_id, std::string const& name) -> std::int32_t {
    return glGetUniformLocation(shader_id, name.c_str());
}

auto shader_compile(std::uint32_t const& type, char const* source) -> std::uint32_t {
    std::uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    std::int32_t is_success;
    constexpr auto LOG_SIZE = 512;
    static char info_log[LOG_SIZE];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &is_success);
    if (!is_success) {
        glGetShaderInfoLog(shader, LOG_SIZE, nullptr, info_log);
        spdlog::error("SHADER::{}SHADER::COMPILE_FAILED {}",
                      type == GL_VERTEX_SHADER ? "VERTEX_" : "FRAGMENT_",
                      info_log);
        throw std::runtime_error("Shader compilation error");
    }

    return shader;
}
auto shader_link(std::uint32_t const& fs, std::uint32_t const& vs) -> std::uint32_t {
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
        spdlog::error("SHADER::LINK {}", info_log);
        throw std::runtime_error("Shader linking error");
    }

    glUseProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}
}

auto main([[maybe_unused]]int argc, [[maybe_unused]]char const* argv[]) -> int {
    std::string title   = "Hello, Text!";
    std::int32_t width  = 738;
    std::int32_t height = 480;

    if (!glfwInit()) {
        spdlog::error("Failed to initialise GLFW!");
        return -1;
    }
    one::setup_opengl();
    GLFWwindow* window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        spdlog::error("Can't create GLFW window!");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
        glfwTerminate();
        spdlog::error("Failed to load glad!\n");
    }

    FT_Library font_library;
    if (FT_Init_FreeType(&font_library)) {
        spdlog::error("Failed initialising FreeType");
        return 1;
    }

    std::string font_path{"deps/fonts/Cozette/CozetteVector.ttf"};
    if (!std::filesystem::exists(font_path)) {
        spdlog::error("Font file: {:s} does not exist!", font_path);
        return 1;
    }
    FT_Face font_face;
    auto font_error = FT_New_Face(font_library, font_path.c_str(), 0, &font_face);
    if (font_error == FT_Err_Unknown_File_Format) {
        spdlog::error("Unknown file format!");
        return 1;
    } else if (font_error) {
        spdlog::error("Failed Loading font!");
        return 1;
    }

    std::unordered_map<std::uint32_t, one::character> characters;
    FT_Set_Pixel_Sizes(font_face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (std::uint32_t code = 0; code < 128; code++) {
        if (FT_Load_Char(font_face, code, FT_LOAD_RENDER)) {
            spdlog::warn("FreeType: Failed to load glyph code: {}", code);
            continue;
        }
        std::uint32_t texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_R8,
                     font_face->glyph->bitmap.width,
                     font_face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     font_face->glyph->bitmap.buffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        characters.insert({code,
                {
                    texture_id,
                    {font_face->glyph->bitmap.width, font_face->glyph->bitmap.rows},
                    {font_face->glyph->bitmap_left,  font_face->glyph->bitmap_top},
                    font_face->glyph->advance.x
                }
        });
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Clean up FreeType resources
    FT_Done_Face(font_face);
    FT_Done_FreeType(font_library);

    glm::mat4 projection = glm::ortho(0.0f, float(width), 0.0f, float(height));

    std::uint32_t array_buffer;
    std::uint32_t vertex_buffer;
    glGenVertexArrays(1, &array_buffer);
    glGenBuffers(1, &vertex_buffer);
    glBindVertexArray(array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 6, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void const*)(2 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    auto render_text = [&](std::uint32_t const& shader_id, std::string const& text, glm::vec2 const& position, float scale, glm::vec3 const& color) {
        glUseProgram(shader_id);
        glUniform3f(one::uniform_name(shader_id, "u_textcolor"), color.r, color.g, color.b);
        glUniform1i(one::uniform_name(shader_id, "u_texture"), 0);
        glBindVertexArray(array_buffer);

        float x = position.x;
        float y = position.y;
        std::for_each(std::begin(text), std::end(text),
        [&](char const& code) {
            auto ch = characters[std::uint32_t(code)];

            auto xpos = x + float(ch.bearing.x) * scale;
            auto ypos = y - float(ch.size.y - ch.bearing.y) * scale;

            auto w = float(ch.size.x) * scale;
            auto h = float(ch.size.y) * scale;

            float vertices[6][4] = {
                {xpos,     ypos + h,   0.0f, 0.0f},
                {xpos,     ypos,       0.0f, 1.0f},
                {xpos + w, ypos,       1.0f, 1.0f},

                {xpos,     ypos + h,   0.0f, 0.0f},
                {xpos + w, ypos,       1.0f, 1.0f},
                {xpos + w, ypos + h,   1.0f, 0.0f},
            };
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ch.texture_id);
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += float(ch.advance >> 6) * scale;
        });
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    };

    // create shader for rendering
    std::uint32_t shader_id = one::shader_link(
            one::shader_compile(GL_VERTEX_SHADER,   one::vertex_shader),
            one::shader_compile(GL_FRAGMENT_SHADER, one::fragment_shader)
        );

    auto is_running = true;
    while(is_running) {
        is_running = !glfwWindowShouldClose(window);
        glfwGetFramebufferSize(window, &width, &height);
        projection = glm::ortho(0.0f, float(width), 0.0f, float(height));

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            is_running = false;

        // Clear screen
        glViewport(0, 0, width, height);
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_id);
        glUniformMatrix4fv(one::uniform_name(shader_id, "u_projection"), 1, GL_FALSE,
                   glm::value_ptr(projection));

        render_text(shader_id, "Hello, World!", {25.0f, 25.0f}, 1.0f, {0.5, 0.8f, 0.2f});

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    std::for_each(std::begin(characters), std::end(characters),
    [](auto const& character){
        glDeleteTextures(1, &character.second.texture_id);
    });

    // Clean up GLFW resource
    glfwTerminate();

    return 0;
}

