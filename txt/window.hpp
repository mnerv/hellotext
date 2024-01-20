#ifndef TXT_WINDOW_HPP
#define TXT_WINDOW_HPP
#include <string_view>
#include <memory>
#include <functional>
#include <string>
#include <filesystem>
#include <chrono>
#include <unordered_map>

#include "glm/vec2.hpp"
#include "utility.hpp"

#include "input.hpp"
#include "event.hpp"

namespace txt {
auto read_text(std::filesystem::path const& filename) -> std::string;

template <typename T>
concept EventFunc = std::is_invocable_r_v<void, T, event const&>;

class window {
public:
    struct props {
        std::string_view  title  = "txt::window";
        std::uint32_t     width  = 960;
        std::uint32_t     height = 600;
    };

    using event_fn  = std::function<void(event const&)>;
    using event_map = std::unordered_map<std::size_t, event_fn>;

public:
    window(window::props const& properties) noexcept;
    ~window() noexcept;

    auto setup() -> void;
    auto width() const noexcept -> std::uint32_t;
    auto height() const noexcept -> std::uint32_t;
    auto buffer_width() const noexcept -> std::uint32_t;
    auto buffer_height() const noexcept -> std::uint32_t;
    auto should_close() const noexcept -> bool;
    auto x() const noexcept -> double;
    auto y() const noexcept -> double;
    auto content_scale_x() const noexcept -> double;
    auto content_scale_y() const noexcept -> double;
    auto is_focused() const noexcept -> bool;
    auto is_hovered() const noexcept -> bool;
    auto is_maximized() const noexcept -> bool;

    auto time() const -> double;
    auto stopwatch() const -> double;
    auto close() -> void;
    auto poll() -> void;
    auto swap() -> void;

    template <typename T>
    requires std::is_invocable_v<T, mouse_move_event const&>
    auto add_event_listener(T const& fn) -> void {
        auto const& id = std::size_t(&fn);
        add_event_listener(event_type::mouse_move, id, [&fn](auto const& e) {
            fn(static_cast<mouse_move_event const&>(e));
        });
    }
    template <typename T>
    requires std::is_invocable_v<T, mouse_move_event const&>
    auto remove_event_listener(T const& fn) -> void {
        remove_event_listener(event_type::mouse_move, std::size_t(&fn));
    }

public:
    auto add_event_listener(event_type const& type, EventFunc auto const& fn) -> void {
        add_event_listener(type, std::size_t(&fn), fn);
    }

    auto remove_event_listener(event_type const& type, EventFunc auto const& fn) -> void {
        remove_event_listener(type, std::size_t(&fn));
    }

private:
    auto add_event_listener(event_type const& type, std::size_t const& id, event_fn const& fn) -> void;
    auto remove_event_listener(event_type const& type, std::size_t const& id) -> void;

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
    double        m_content_scale_x{1.0};
    double        m_content_scale_y{1.0};
    std::int32_t  m_position_x{0};
    std::int32_t  m_position_y{0};
    bool          m_is_focused{true};
    bool          m_is_maximized{false};
    bool          m_is_hovered{false};
    double        m_mouse_x{0.0};
    double        m_mouse_y{0.0};
    std::unordered_map<event_type, event_map> m_listeners{};

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
