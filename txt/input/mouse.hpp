#ifndef TXT_INPUT_MOUSE_HPP
#define TXT_INPUT_MOUSE_HPP

#include <cstdint>

namespace txt {
enum class mouse_button : std::uint32_t {
    B1     = 0,
    B2     = 1,
    B3     = 2,
    B4     = 3,
    B5     = 4,
    B6     = 5,
    B7     = 6,
    B8     = 7,
    left   = B1,
    right  = B2,
    middle = B3,
};
}

#endif  // TXT_INPUT_MOUSE_HPP
