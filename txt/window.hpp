#ifndef TXT_WINDOW_HPP
#define TXT_WINDOW_HPP
#include <string_view>
#include <memory>
#include <functional>
#include <string>
#include <filesystem>
#include <chrono>

#include "glm/vec2.hpp"
#include "utility.hpp"

namespace txt {
auto read_text(std::filesystem::path const& filename) -> std::string;

class window {
public:
    struct props {
        std::string_view title  = "txt::window";
        std::uint32_t     width  = 960;
        std::uint32_t     height = 600;
    };

public:
    window(window::props const& props);
    ~window();

    auto width() const noexcept -> std::uint32_t;
    auto height() const noexcept -> std::uint32_t;
    auto buffer_width() const noexcept -> std::uint32_t;
    auto buffer_height() const noexcept -> std::uint32_t;
    auto should_close() const noexcept -> bool;

    auto time() const -> double;
    auto stopwatch() const -> double;
    auto close() -> void;
    auto poll() -> void;
    auto swap() -> void;

private:
    auto setup_native() -> void;
    auto clean_native() -> void;

private:
    std::string   m_title;
    std::uint32_t m_width;
    std::uint32_t m_height;
    std::uint32_t m_buffer_width;
    std::uint32_t m_buffer_height;
    bool          m_should_close{false};

private:
    void* m_native{nullptr};
};

using window_ref_t = ref<window>;
auto make_window(window::props const& props) -> window_ref_t;

using loop_t = std::function<void()>;
auto loop(window_ref_t window, loop_t fn) -> void;

using loop_dt_t = std::function<void(double)>;
auto loop(window_ref_t window, loop_dt_t fn) -> void;
} // namespace txt

#endif  // TXT_WINDOW_HPP
