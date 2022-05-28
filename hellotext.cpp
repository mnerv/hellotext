#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>

#include "spdlog/spdlog.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#include "glm/glm.hpp"

namespace one {
static auto setup_opengl() -> void {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

[[maybe_unused]]static auto vertex_shader = R"(#version 410 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_uv;

out vec4 io_color;
out vec2 io_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    io_color = a_color;
    io_uv    = a_uv;

    gl_Position = vec4(a_position, 1.0f);
}
)";

[[maybe_unused]]static auto fragment_shader = R"(#version 410 core
layout(location = 0) out vec4 color;

in vec4 io_color;
in vec2 io_uv;

uniform vec4 u_textcolor;
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
    std::uint32_t advance;     // Offset to advance to next glyph
};
}

auto main([[maybe_unused]]std::int32_t argc, [[maybe_unused]]char const* argv[]) -> std::int32_t {
    std::string title   = "Hello, Text!";
    std::int32_t width  = 738;
    std::int32_t height = 480;

    if (!glfwInit())
        return -1;
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

    std::string font_path{"/Users/k/Downloads/CozetteVector.otf"};
    FT_Face font_face;
    auto font_error = FT_New_Face(font_library, font_path.c_str(), 0, &font_face);
    if (font_error == FT_Err_Unknown_File_Format) {
        spdlog::error("Unknown file format!");
        return 1;
    } else if (font_error) {
        spdlog::error("Failed Loading font!");
        return 1;
    }
    FT_Set_Pixel_Sizes(font_face, 0, 48);
    if (FT_Load_Char(font_face, 'X', FT_LOAD_RENDER)) {
        spdlog::error("Failed to load Glyph");
        return 1;
    }

    auto is_running = true;
    while(is_running) {
        is_running = !glfwWindowShouldClose(window);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            is_running = false;

        // Clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up FreeType resources
    FT_Done_Face(font_face);
    FT_Done_FreeType(font_library);

    // Clean up GLFW resource
    glfwTerminate();

    return 0;
}

