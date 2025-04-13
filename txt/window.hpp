#ifndef TXT_WINDOW_HPP
#define TXT_WINDOW_HPP
#include <string_view>
#include <functional>
#include <string>
#include <filesystem>
#include <unordered_map>

#include "asio.hpp"

#include "types.hpp"
#include "utils.hpp"
#include "event.hpp"

namespace txt {
auto read_text(std::filesystem::path const& filename) -> std::string;
auto graphics_info() -> std::string;

// SOURCE: https://stackoverflow.com/questions/6512019/can-we-get-the-type-of-a-lambda-argument
template<typename Ret, typename Arg, typename... Rest>
Arg arg1_helper(Ret(*) (Arg, Rest...));
template<typename Ret, typename F, typename Arg, typename... Rest>
Arg arg1_helper(Ret(F::*) (Arg, Rest...));
template<typename Ret, typename F, typename Arg, typename... Rest>
Arg arg1_helper(Ret(F::*) (Arg, Rest...) const);
template <typename F>
decltype(arg1_helper(&F::operator())) arg1_helper(F);

template <typename T>
using arg1_t = decltype(arg1_helper(std::declval<T>()));

template <typename T>
concept EventFunc = std::is_invocable_r_v<void, T, event const&>;

class window {
public:
    struct props {
        std::string_view title  = "txt::window";
        u32              width  = 960;
        u32              height = 600;
    };

    using event_fn  = std::function<void(event const&)>;
    using event_map = std::unordered_map<std::size_t, event_fn>;

public:
    window(window::props const& properties) noexcept;
    ~window() noexcept;

    auto setup() -> void;
    auto fullscreen() -> void;
    auto exit_fullscreen() -> void;
    auto toggle_fullscreen() -> void;

    auto width() const noexcept -> u32;
    auto height() const noexcept -> u32;
    auto buffer_width() const noexcept -> u32;
    auto buffer_height() const noexcept -> u32;
    auto should_close() const noexcept -> bool;
    auto x() const noexcept -> f64;
    auto y() const noexcept -> f64;
    auto content_scale_x() const noexcept -> f64;
    auto content_scale_y() const noexcept -> f64;
    auto is_focused() const noexcept -> bool;
    auto is_hovered() const noexcept -> bool;
    auto is_fullscreen() const noexcept -> bool;
    auto is_init() const noexcept -> bool;

    auto time() const -> f64;
    auto stopwatch() const -> f64;
    auto close() -> void;
    auto poll() -> void;
    auto swap() -> void;

    template <typename T>
    auto add_event_listener(T const& fn) -> void {
        using FnArgT = arg1_t<T>;
        using ArgT = std::remove_const_t<std::remove_reference_t<FnArgT>>;
        static_assert(std::is_base_of_v<txt::event, ArgT>);
        static_assert(!std::is_same_v<txt::event, ArgT>, "Function type can't be same as event");
        constexpr auto type = event_t_to_enum<ArgT>();
        static_assert(type != event_type::none, "Unknown event type");

        auto const& id = std::size_t(&fn);
        add_event_listener(type, id, [&fn](auto const& e) {
            fn(static_cast<FnArgT>(e));
        });
    }
    template <typename T>
    auto remove_event_listener(T const& fn) -> void {
        using FnArgT = arg1_t<T>;
        using ArgT = std::remove_const_t<std::remove_reference_t<FnArgT>>;
        static_assert(std::is_base_of_v<txt::event, ArgT>);
        static_assert(!std::is_same_v<txt::event, ArgT>, "Function type can't be same as event");
        constexpr auto type = event_t_to_enum<ArgT>();
        static_assert(type != event_type::none, "Unknown event type");

        remove_event_listener(type, std::size_t(&fn));
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
    auto poll_native() -> void;
    auto swap_native() -> void;
    auto fullscreen_native() -> void;
    auto exit_fullscreen_native() -> void;

private:
    std::string m_title;
    u32  m_width;
    u32  m_height;
    u32  m_buffer_width;
    u32  m_buffer_height;
    bool m_should_close{false};
    f64  m_content_scale_x{1.0};
    f64  m_content_scale_y{1.0};
    i32  m_position_x{0};
    i32  m_position_y{0};
    bool m_is_focused{true};
    bool m_is_fullscreen{false};
    bool m_is_hovered{false};
    f64  m_mouse_x{0.0};
    f64  m_mouse_y{0.0};
    std::unordered_map<event_type, event_map> m_listeners{};
    bool m_is_init{false};

private:
    asio::io_context m_asio{};
    asio::io_context::work m_idle{m_asio};
    struct {
        u32  m_width{0};
        u32  m_height{0};
        i32  m_position_x{0};
        i32  m_position_y{0};
    } m_windowed;

private:
    void* m_native{nullptr};
};

using window_ref_t = ref<window>;
auto make_window(window::props const& props) -> window_ref_t;

using loop_fn_t = std::function<void()>;
auto loop(window_ref_t ctx, loop_fn_t fn) -> void;
} // namespace txt

#endif  // TXT_WINDOW_HPP
