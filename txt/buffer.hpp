#ifndef TXT_BUFFER_HPP
#define TXT_BUFFER_HPP

#include <cstddef>
#include <cstdint>

namespace txt {
class buffer {
public:
    buffer();
    buffer(void const* data, std::size_t bytes, std::size_t size, GLenum type, std::uint32_t usage);
    ~buffer();

    auto bind() -> void;
    auto unbind() -> void;

    auto id() const -> std::uint32_t;
    auto bytes() const -> std::size_t;
    auto size() const -> std::size_t;

    auto set(void const* data, std::size_t bytes, std::size_t size, GLenum type, std::uint32_t usage) -> void;
    auto resize(std::size_t bytes) -> void;
    auto sub(void const* data, std::size_t bytes, std::size_t offset = 0) -> void;

private:
    std::uint32_t m_id;
    std::size_t   m_bytes;
    std::size_t   m_size;
    std::uint32_t m_type;
    std::uint32_t m_usage;
};
} // namespace txt

#endif  // TXT_BUFFER_HPP
