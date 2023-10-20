#ifndef DEMOS_APP_HPP
#define DEMOS_APP_HPP

#include "txt/window.hpp"

namespace demo {
class app {
public:
    app(txt::window_ref_t window) : m_window(window) { }
    virtual ~app() = default;

    virtual auto update(double dt) -> void = 0;

protected:
    txt::window_ref_t m_window;
};
}
#endif  // DEMOS_APP_HPP
