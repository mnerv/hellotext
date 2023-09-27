#ifndef TXT_SHADER_HPP
#define TXT_SHADER_HPP
#include <cstdint>
#include <string>

#include "utility.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat2x2.hpp"
#include "glm/mat3x3.hpp"
#include "glm/mat4x4.hpp"

namespace txt {
class shader {
public:
    shader(std::string const& vs_src, std::string const& fs_src);
    ~shader();

    auto bind() -> void;
    auto unbind() -> void;
    auto id() const -> std::uint32_t;

public:
    auto upload_num(std::string const& name, std::uint32_t const& value) -> void;
    auto upload_num(std::string const& name, std::int32_t const& value) -> void;
    auto upload_num(std::string const& name, float const& value) -> void;

    auto upload_nums(std::string const& name, std::int32_t const& count, std::uint32_t const* values) -> void;
    auto upload_nums(std::string const& name, std::int32_t const& count, float const* values) -> void;

    auto upload_vec2(std::string const& name, glm::vec2 const& value) -> void;
    auto upload_vec3(std::string const& name, glm::vec3 const& value) -> void;
    auto upload_vec4(std::string const& name, glm::vec4 const& value) -> void;

    auto upload_vec2s(std::string const& name, std::int32_t const& count, glm::vec2 const* values) -> void;
    auto upload_vec3s(std::string const& name, std::int32_t const& count, glm::vec3 const* values) -> void;
    auto upload_vec4s(std::string const& name, std::int32_t const& count, glm::vec4 const* values) -> void;

    auto upload_mat2(std::string const& name, glm::mat2 const& value, bool const& transpose = false) -> void;
    auto upload_mat3(std::string const& name, glm::mat3 const& value, bool const& transpose = false) -> void;
    auto upload_mat4(std::string const& name, glm::mat4 const& value, bool const& transpose = false) -> void;

    auto upload_mat2s(std::string const& name, std::int32_t const& count, glm::mat2 const* values, bool const& transpose = false) -> void;
    auto upload_mat3s(std::string const& name, std::int32_t const& count, glm::mat3 const* values, bool const& transpose = false) -> void;
    auto upload_mat4s(std::string const& name, std::int32_t const& count, glm::mat4 const* values, bool const& transpose = false) -> void;

private:
    [[nodiscard]] auto uniform_location(std::string const& name) const -> std::int32_t;

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
