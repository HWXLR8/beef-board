#pragma once

#include <optional>

#include "rgb.h"

constexpr auto FLASH_CONFIG_OFFSET = 8 * 1024 * 1024;

enum class ControllerType : uint8_t
{
    IIDX,
    SDVX
};

enum class InputMode : uint8_t
{
    Joystick,
    Keyboard
};

// Key mapping structures
struct IIDXKeyMapping
{
    uint8_t key_codes[13];
    uint8_t reserved[7];

    friend bool operator==(const IIDXKeyMapping &lhs, const IIDXKeyMapping &rhs)
    {
        for (int i = 0; i < 13; i++)
        {
            if (lhs.key_codes[i] != rhs.key_codes[i])
                return false;
        }
        return true;
    }
};

struct SDVXKeyMapping
{
    uint8_t key_codes[9];
    uint8_t reserved[11];

    friend bool operator==(const SDVXKeyMapping &lhs, const SDVXKeyMapping &rhs)
    {
        for (int i = 0; i < 9; i++)
        {
            if (lhs.key_codes[i] != rhs.key_codes[i])
                return false;
        }
        return true;
    }
};

enum class TurntableMode : uint8_t
{
    Static,
    Spin,
    Shift,
    RainbowStatic,
    RainbowReact,
    RainbowSpin,
    React,
    Breathing,
    HID,
    Disable,
    Count
};

enum class BarMode : uint8_t
{
    Placeholder1, // beat
    KeySpectrumP1,
    KeySpectrumP2,
    Placeholder3, // audio spectrum
    HID,
    TapeLedP1,
    TapeLedP2,
    Disable,
    Count
};

struct callback_t
{
    bool deferred_save;
    uint8_t val;
};

// Do not reorder these fields
struct config_t
{
    uint32_t magic;
    uint8_t version;
    bool reverse_tt;
    TurntableMode tt_effect;
    uint8_t tt_deadzone;
    BarMode bar_effect;
    bool disable_leds;
    hsv_t tt_static_hsv;
    hsv_t tt_spin_hsv;
    hsv_t tt_shift_hsv;
    hsv_t tt_rainbow_static_hsv;
    hsv_t tt_rainbow_react_hsv;
    hsv_t tt_rainbow_spin_hsv;
    hsv_t tt_react_hsv;
    hsv_t tt_breathing_hsv;
    uint8_t tt_ratio;
    ControllerType controller_type;
    InputMode iidx_input_mode;
    InputMode sdvx_input_mode;
    uint8_t tt_sustain_ms;
    IIDXKeyMapping iidx_keys;
    SDVXKeyMapping sdvx_keys;
    uint8_t iidx_buttons_debounce;
    uint8_t iidx_effectors_debounce;
    uint8_t sdvx_buttons_debounce;
    uint8_t led_refresh;
    uint8_t rainbow_spin_speed;
    uint8_t tt_leds;

    void save() const;
    // void set_controller_type(ControllerType mode);
    // void set_input_mode(InputMode mode);

    bool equals(const config_t* other) const
    {
        return version == other->version &&
            reverse_tt == other->reverse_tt &&
            tt_effect == other->tt_effect &&
            tt_deadzone == other->tt_deadzone &&
            bar_effect == other->bar_effect &&
            disable_leds == other->disable_leds &&
            tt_static_hsv == other->tt_static_hsv &&
            tt_spin_hsv == other->tt_spin_hsv &&
            tt_shift_hsv == other->tt_shift_hsv &&
            tt_rainbow_static_hsv == other->tt_rainbow_static_hsv &&
            tt_rainbow_react_hsv == other->tt_rainbow_react_hsv &&
            tt_rainbow_spin_hsv == other->tt_rainbow_spin_hsv &&
            tt_react_hsv == other->tt_react_hsv &&
            tt_breathing_hsv == other->tt_breathing_hsv &&
            tt_ratio == other->tt_ratio &&
            controller_type == other->controller_type &&
            iidx_input_mode == other->iidx_input_mode &&
            sdvx_input_mode == other->sdvx_input_mode &&
            tt_sustain_ms == other->tt_sustain_ms &&
            iidx_keys == other->iidx_keys &&
            sdvx_keys == other->sdvx_keys &&
            iidx_buttons_debounce == other->iidx_buttons_debounce &&
            iidx_effectors_debounce == other->iidx_effectors_debounce &&
            sdvx_buttons_debounce == other->sdvx_buttons_debounce &&
            led_refresh == other->led_refresh &&
            rainbow_spin_speed == other->rainbow_spin_speed &&
            tt_leds == other->tt_leds;
    }
};

extern config_t config;
void config_init();

std::optional<callback_t> toggle_reverse_tt();
std::optional<callback_t> cycle_tt_effects();
// std::optional<callback_t> tt_hsv_set_hue();
// std::optional<callback_t> tt_hsv_set_sat();
// std::optional<callback_t> tt_hsv_set_val();
std::optional<callback_t> increase_deadzone();
std::optional<callback_t> decrease_deadzone();
std::optional<callback_t> increase_ratio();
std::optional<callback_t> decrease_ratio();
std::optional<callback_t> cycle_bar_effects();
std::optional<callback_t> toggle_disable_leds();
