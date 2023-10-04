#ifndef TXT_UTILITY_HPP
#define TXT_UTILITY_HPP
#include <memory>
#include <cstddef>
#include <limits>
#include <functional>
#include <type_traits>
#include <concepts>
#include <numbers>

namespace txt {

template <typename T>
concept Integral = std::is_integral<T>::value;

template <typename T, std::size_t N>
inline constexpr auto len(T (&)[N]) -> std::size_t {
    return N;
}
inline constexpr auto bit_on(Integral auto const& position) {
    return 1 << position;
}
inline constexpr auto bit_level(Integral auto const& reg, Integral auto const& mask, Integral auto const& data) {
    return (reg & ~mask) | (data & mask);
}

template <typename T>
using limits = std::numeric_limits<T>;

template <typename T>
using ref = std::shared_ptr<T>;
template <typename T>
using local = std::unique_ptr<T>;
template <typename T>
using weak = std::weak_ptr<T>;

template <typename T, typename... Args>
constexpr auto make_ref(Args&&... args) -> ref<T> {
    return std::make_shared<T>(std::forward<Args>(args)...);
}
template <typename T, typename... Args>
constexpr auto make_local(Args&&... args) -> local<T> {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// Math constants
inline constexpr auto pi    = std::numbers::pi;
inline constexpr auto pif32 = std::numbers::pi_v<float>;
} // namespace txt

#endif // TXT_UTILITY_HPP

