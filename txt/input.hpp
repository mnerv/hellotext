#ifndef TXT_INPUT_HPP
#define TXT_INPUT_HPP
#include <cstdint>

#define TXT_INPUT_ENUM_MODIFIERS            \
        TXT_INPUT_ENUM_MOD(shift, 0)        \
        TXT_INPUT_ENUM_MOD(control, 1)      \
        TXT_INPUT_ENUM_MOD(alternative, 2)  \
        TXT_INPUT_ENUM_MOD(super, 3)        \
        TXT_INPUT_ENUM_MOD(caps_lock, 4)    \
        TXT_INPUT_ENUM_MOD(num_lock, 5)

namespace txt {
enum class mod_flag : std::uint32_t {
#define TXT_INPUT_ENUM_MOD(name, pos) name = (1 << pos),
    TXT_INPUT_ENUM_MODIFIERS
#undef TXT_INPUT_ENUM_MOD
};

class modifier_flags {
public:
    enum class flag_index : std::uint32_t {
#define TXT_INPUT_ENUM_MOD(name, pos) name = pos,
        TXT_INPUT_ENUM_MODIFIERS
#undef TXT_INPUT_ENUM_MOD
    };

public:
    modifier_flags(std::uint32_t const& flags) : m_flags(flags) {}
    ~modifier_flags() = default;

    [[nodiscard]]auto shift() const -> bool { return flag_state(flag_index::shift); }
    [[nodiscard]]auto control() const -> bool { return flag_state(flag_index::control); }
    [[nodiscard]]auto alternative() const -> bool { return flag_state(flag_index::alternative); }
    [[nodiscard]]auto super() const -> bool { return flag_state(flag_index::super); }
    [[nodiscard]]auto caps_lock() const -> bool { return flag_state(flag_index::caps_lock); }
    [[nodiscard]]auto num_lock() const -> bool { return flag_state(flag_index::num_lock); }
    [[nodiscard]]auto raw() const -> std::uint32_t { return m_flags; }

private:
    auto flag_state(flag_index const& index) const -> bool {
        return static_cast<bool>((m_flags >> std::uint32_t(index)) & 0x1);
    }

private:
    std::uint32_t m_flags{0x00};
};
}
#endif  // TXT_INPUT_HPP
