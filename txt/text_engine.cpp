#include "text_engine.hpp"
#include "renderer.hpp"
#include "utf8.h"

namespace txt {
[[maybe_unused]]static constexpr float quad_vertices[]{
//     x,     y,     z,       u,    v,
    0.0f,  0.0f,  0.0f,    0.0f, 0.0f,
    0.0f,  1.0f,  0.0f,    0.0f, 1.0f,
    1.0f,  1.0f,  0.0f,    1.0f, 1.0f,
    1.0f,  0.0f,  0.0f,    1.0f, 0.0f,
};
[[maybe_unused]]static constexpr std::uint32_t quad_cw_indices[]{
    0, 1, 2,
    0, 2, 3
};
} // namespace txt
