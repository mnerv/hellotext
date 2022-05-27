#include <cstdint>
#include <iostream>
#include <string>

#include "ft2build.h"
#include FT_FREETYPE_H

#include "spdlog/spdlog.h"

auto main([[maybe_unused]]std::int32_t argc, [[maybe_unused]]char const* argv[]) -> std::int32_t {
    std::cout << "Hello, Text Rendering!\n";

    FT_Library font_library;
    if (FT_Init_FreeType(&font_library)) {
        spdlog::error("Error initialising FreeType");
        return 1;
    }

    std::string font_path{"/Users/k/Downloads/CozetteVector.otf"};
    FT_Face font_face;
    auto font_error = FT_New_Face(font_library, font_path.c_str(), 0, &font_face);
    if (font_error == FT_Err_Unknown_File_Format) {
        spdlog::error("Unknown file format!");
        return 1;
    } else if (font_error) {
        spdlog::error("Error loading font!");
        return 1;
    }

    spdlog::info("glyphs count: {}", font_face->num_glyphs);
    FT_Done_Face(font_face);
    FT_Done_FreeType(font_library);

    return 0;
}

