#ifndef TXT_TEXT_ENGINE_HPP
#define TXT_TEXT_ENGINE_HPP

#include <map>

#include "utility.hpp"
#include "window.hpp"
#include "image.hpp"
#include "fonts.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "buffer.hpp"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace txt {
class text_engine {
public:
    text_engine(window_ref_t window, font_manager_ref_t manager);
    ~text_engine() = default;
};

using text_engine_ref_t = ref<text_engine>;
} // namespace txt

#endif  // TXT_TEXT_ENGINE_HPP
