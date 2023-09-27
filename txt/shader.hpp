#ifndef TXT_SHADER_HPP
#define TXT_SHADER_HPP
#include <cstdint>
#include <string>

#include "utility.hpp"

namespace txt {
class shader {
public:
    shader(std::string const& vs_src, std::string const& fs_src);
    ~shader();

    auto bind() -> void;
    auto unbind() -> void;
    auto id() const -> std::uint32_t;

private:
    static auto compile(std::uint32_t const& type, char const* source) -> std::uint32_t;
    static auto link(std::uint32_t const& vs, std::uint32_t const& fs) -> std::uint32_t;

private:
    std::uint32_t m_id;
};

using shader_ref_t = ref<shader>;
auto make_shader(std::string const& vs_src, std::string const& fs_src) -> shader_ref_t;
} // namespace txt

#endif  // TXT_SHADER_HPP
