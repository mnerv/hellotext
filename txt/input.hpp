#ifndef TXT_INPUT_HPP
#define TXT_INPUT_HPP
#include <cstdint>

namespace txt {
enum class keycode : std::uint32_t {
    A = 0x41,
    B = 0x42,
    C = 0x43,
    D = 0x44,
    E = 0x45,
    F = 0x46,
    G = 0x47,
    H = 0x48,
    I = 0x49,
    J = 0x4A,
    K = 0x4B,
    L = 0x4C,
    M = 0x4D,
    N = 0x4E,
    O = 0x4F,
    P = 0x50,
    Q = 0x51,
    R = 0x52,
    S = 0x53,
    T = 0x54,
    U = 0x55,
    V = 0x56,
    W = 0x57,
    X = 0x58,
    Y = 0x59,
    Z = 0x5A,
};

// See USB HID Specification: HID Usages and Descriptors under HID Usage Tables
// Keyboard/Keypad Page (0x07)
// https://www.usb.org/hid
enum class scancode : std::uint16_t {
    reserved        = 0x00,
    error_roll_over = 0x01,
    post_fail       = 0x02,
    error_undefined = 0x03,

    // Keyboard a-z
    a = 0x04,
    b = 0x05,
    c = 0x06,
    d = 0x07,
    e = 0x08,
    f = 0x09,
    g = 0x0A,
    h = 0x0B,
    i = 0x0C,
    j = 0x0D,
    k = 0x0E,
    l = 0x0F,
    m = 0x10,
    n = 0x11,
    o = 0x12,
    p = 0x13,
    q = 0x14,
    r = 0x15,
    s = 0x16,
    t = 0x17,
    u = 0x18,
    v = 0x19,
    w = 0x1A,
    x = 0x1B,
    y = 0x1C,
    z = 0x1D,

    // Keyboard 0-9
    n1 = 0x1E,
    n2 = 0x1F,
    n3 = 0x20,
    n4 = 0x21,
    n5 = 0x22,
    n6 = 0x23,
    n7 = 0x24,
    n8 = 0x25,
    n9 = 0x26,
    n0 = 0x27,

    // Controls
    enter             = 0x28,  // Return
    escape            = 0x29,
    del               = 0x2A,  // DELETE (Backspace)
    tab               = 0x2B,
    space             = 0x2C,
    underscore        = 0x2D,
    equal             = 0x2E,  // = and +

    // Marker
    left_sqr_bracket  = 0x2F,  // [ and {
    right_sqr_bracket = 0x30,  // ] and }
    backward_slash    = 0x31,  // \ (Backward slash) and |
    pound_key         = 0x32,  // # and ~
    semicolon         = 0x33,  // ; and :
    apostrophe        = 0x34,  // ' and "
    grave_accent      = 0x35,  // Grave Accent and Tilde
    comma             = 0x36,  // , and <
    dot               = 0x37,  // . and >
    slash             = 0x38,  // / and ?
    // forward_slash     = 0x38,  // / and ?
    caps_lock         = 0x39,  // Caps Lock

    // Function keys
    f1  = 0x3A,
    f2  = 0x3B,
    f3  = 0x3C,
    f4  = 0x3D,
    f5  = 0x3E,
    f6  = 0x3F,
    f7  = 0x40,
    f8  = 0x41,
    f9  = 0x42,
    f10 = 0x43,
    f11 = 0x44,
    f12 = 0x45,

    // Break key
    print_screen   = 0x46,  // PrintScreen
    scroll_lock    = 0x47,  // Scroll Lock
    pause          = 0x48,
    insert         = 0x49,
    home           = 0x4A,
    page_up        = 0x4B,
    delete_forward = 0x4C,
    end            = 0x4D,
    page_down      = 0x4E,
    right_arrow    = 0x4F,
    left_arrow     = 0x50,
    down_arrow     = 0x51,
    up_arrow       = 0x52,
    num_lock       = 0x53,  // Num Lock and Clear

    // Keypad
    kp_slash    = 0x54,  // Keypad /
    kp_asterisk = 0x55,  // Keypad *
    kp_minus    = 0x56,  // Keypad -
    kp_plus     = 0x57,  // Keypad -
    kp_enter    = 0x58,
    kp_1        = 0x59,  // Keypad 1 and End
    kp_2        = 0x5A,  // Keypad 2 and Down Arrow
    kp_3        = 0x5B,  // Keypad 3 and Page Down
    kp_4        = 0x5C,  // Keypad 4 and Left Arrow
    kp_5        = 0x5D,  // Keypad 5
    kp_6        = 0x5E,  // Keypad 6 and Right Arrow
    kp_7        = 0x5F,  // Keypad 7 and Home
    kp_8        = 0x60,  // Keypad 8 and Up Arrow
    kp_9        = 0x61,  // Keypad 9 and Page Up
    kp_0        = 0x62,  // Keypad 0 and Insert
    kp_dot      = 0x63,  // Keypad . and Delete

    // Marker 2
    bk_slash    = 0x64,  // \ and |
    application = 0x65,
    power       = 0x66,

    // Keypad
    kp_equal = 0x67,  // Keypad =

    // Function keys 2
    f13 = 0x68,
    f14 = 0x69,
    f15 = 0x6A,
    f16 = 0x6B,
    f17 = 0x6C,
    f18 = 0x6D,
    f19 = 0x6E,
    f20 = 0x6F,
    f21 = 0x70,
    f22 = 0x71,
    f23 = 0x72,
    f24 = 0x73,

    // Media and Controls
    execute     = 0x74,
    help        = 0x75,
    menu        = 0x76,
    select      = 0x77,
    stop        = 0x78,
    again       = 0x79,
    undo        = 0x7A,
    cut         = 0x7B,
    copy        = 0x7C,
    paste       = 0x7D,
    find        = 0x7E,
    mute        = 0x7F,
    volume_up   = 0x80,
    volume_down = 0x81,

    // Locks
    locking_caps_lock   = 0x82,
    locking_num_lock    = 0x83,
    locking_scroll_lock = 0x84,

    // Keypad
    kp_comma      = 0x85,
    kp_equal_sign = 0x86,

    // International
    int1 = 0x87,
    int2 = 0x88,
    int3 = 0x89,
    int4 = 0x8A,
    int5 = 0x8B,
    int6 = 0x8C,
    int7 = 0x8D,
    int8 = 0x8E,
    int9 = 0x8F,

    // Language
    lang1 = 0x90,
    lang2 = 0x91,
    lang3 = 0x92,
    lang4 = 0x93,
    lang5 = 0x94,
    lang6 = 0x95,
    lang7 = 0x96,
    lang8 = 0x97,
    lang9 = 0x98,

    // Controls
    alt_erase  = 0x99,  // Alternative Erase
    attention  = 0x9A,  // SysReq/Attention
    cancel     = 0x9B,
    clear      = 0x9C,
    prior      = 0x9D,
    return_key = 0x9E,
    separator  = 0x9F,
    out        = 0xA0,
    oper       = 0xA1,
    again_2    = 0xA2,
    props      = 0xA3,
    ex_sel     = 0xA4,

    // Reserved 0xA5-0xAF

    // Keypad
    kp_00  = 0xB0,
    kp_000 = 0xB1,

    // Separators
    thousand_separator   = 0xB2,
    decimal_separator    = 0xB3,
    currency_unit        = 0xB4,
    currency_sub_unit    = 0xB5,
    kp_left_parentheses  = 0xB6,  // Keypad (
    kp_right_parentheses = 0xB7,  // Keypad )
    kp_left_braces       = 0xB8,  // Keypad {
    kp_right_braces      = 0xB9,  // Keypad }
    kp_tab               = 0xBA,
    kp_backspace         = 0xBB,

    // Keypad
    kp_A               = 0xBC,
    kp_B               = 0xBD,
    kp_C               = 0xBE,
    kp_D               = 0xBF,
    kp_E               = 0xC0,
    kp_F               = 0xC1,
    kp_xor             = 0xC2,
    kp_wedge           = 0xC3,  // Keypad ^
    kp_percent         = 0xC4,
    kp_left_chevrons   = 0xC5,  // Keypad <
    kp_left_right      = 0xC6,  // Keypad >
    kp_apersand        = 0xC7,  // Keypad &
    kp_double_apersand = 0xC8,  // Keypad &&
    kp_verbar          = 0xC9,  // Keypad |
    kp_double_verbar   = 0xCA,  // Keypad ||
    kp_colon           = 0xCB,  // Keypad :
    kp_pound           = 0xCC,  // Keypad #
    kp_space           = 0xCD,
    kp_at              = 0xCE,  // Keypad @
    kp_excl            = 0xCF,  // Keypad !
    kp_mem_store       = 0xD0,  // Keypad Memory Store
    kp_mem_recall      = 0xD1,  // Keypad Memory Recall
    kp_mem_clear       = 0xD2,  // Keypad Memory Clear
    kp_mem_add         = 0xD3,  // Keypad Memory Add
    kp_mem_subtract    = 0xD4,  // Keypad Memory Subtract
    kp_mem_multiply    = 0xD5,  // Keypad Memory Multiply
    kp_mem_divide      = 0xD6,  // Keypad Memory Divide
    kp_plus_minus      = 0xD7,  // Keypad +/-
    kp_clear           = 0xD8,
    kp_clear_entry     = 0xD9,
    kp_binary          = 0xDA,
    kp_octal           = 0xDB,
    kp_decimal         = 0xDC,
    kp_hexadecimal     = 0xDD,

    // Reserved 0xDE-0xDF

    // Modifiers
    left_control  = 0xE0,
    left_shift    = 0xE1,
    left_alt      = 0xE2,
    left_gui      = 0xE3,
    right_control = 0xE4,
    right_shift   = 0xE5,
    right_alt     = 0xE6,
    right_gui     = 0xE7,

    // Reserve from 0xE8 to 0xFFFF
    reserved_end = 0xE8
};

class modifier_flags {
public:
    enum class flag : std::uint32_t {
        alpha_shift = 0,
        shift       = 1,
        control     = 2,
        alternative = 3,
        super       = 4,
        caps_lock   = 5,
        num_lock    = 6,
    };

public:
    modifier_flags(std::uint32_t const& flags) : m_flags(flags) {}
    ~modifier_flags() = default;

    [[nodiscard]]auto alpha_shift() const -> bool { return flag_state(flag::alpha_shift); }
    [[nodiscard]]auto shift() const -> bool { return flag_state(flag::shift); }
    [[nodiscard]]auto control() const -> bool { return flag_state(flag::control); }
    [[nodiscard]]auto alternative() const -> bool { return flag_state(flag::alternative); }
    [[nodiscard]]auto super() const -> bool { return flag_state(flag::super); }
    [[nodiscard]]auto caps_lock() const -> bool { return flag_state(flag::caps_lock); }
    [[nodiscard]]auto num_lock() const -> bool { return flag_state(flag::num_lock); }
    [[nodiscard]]auto raw() const -> std::uint32_t { return m_flags; }

private:
    auto flag_state(flag const& index) const -> bool {
        return static_cast<bool>((m_flags >> std::uint32_t(index)) & 0x1);
    }

private:
    std::uint32_t m_flags{0x00};
};
}
#endif  // TXT_INPUT_HPP
