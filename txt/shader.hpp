#ifndef TXT_SHADER_HPP
#define TXT_SHADER_HPP
#include <cstdint>

namespace txt {
class shader {
public:

private:
    auto shader_compile(std::uint32_t const& type, char const* source) -> std::uint32_t;
    auto shader_link(std::uint32_t const& vs, std::uint32_t const& fs) -> std::uint32_t;

private:
};
} // namespace txt

#endif  // TXT_SHADER_HPP
