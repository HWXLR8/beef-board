#include <cstring>

#include "beef.h"
#include "config.h"
#include "class/hid/hid.h"
#include "hardware/flash.h"
#include "pico/flash.h"

constexpr auto DEFAULT_COLOUR = HSV{ 128, 255, 255 }; // Aqua
constexpr auto DEFAULT_COLOUR_NO_HUE = HSV{ 0, 255, 255 };
constexpr uint32_t magic = 0xDEADBEEF;

config_t config{};
const auto* config_mm = reinterpret_cast<const config_t*>(XIP_BASE + FLASH_CONFIG_OFFSET);

constexpr IIDXKeyMapping DEFAULT_IIDX_KEYS = {
    .key_codes = {
        HID_KEY_S,          // 1
        HID_KEY_D,          // 2
        HID_KEY_F,          // 3
        HID_KEY_SPACE,      // 4
        HID_KEY_J,          // 5
        HID_KEY_K,          // 6
        HID_KEY_L,          // 7
        HID_KEY_1,          // E1/Start
        HID_KEY_2,          // E2
        HID_KEY_3,          // E3
        HID_KEY_4,          // E4/Select
        HID_KEY_ARROW_DOWN, // TT -
        HID_KEY_ARROW_UP    // TT +
    }
};

constexpr SDVXKeyMapping DEFAULT_SDVX_KEYS = {
    .key_codes = {
        HID_KEY_D, // BT-A
        HID_KEY_F, // BT-B
        HID_KEY_J, // BT-C
        HID_KEY_K, // BT-D
        HID_KEY_C, // FX-L
        HID_KEY_M, // FX-R
        HID_KEY_NONE,
        HID_KEY_NONE,
        HID_KEY_ENTER // Start
    }
};

struct flash_op_params
{
    uint32_t offset;
    const uint8_t* data;
    size_t count;
};

constexpr size_t round_up_to(const size_t multiple, const size_t v)
{
    const auto remainder = multiple - (v % multiple);
    return v + (remainder % multiple);
}

constexpr size_t config_count_rounded = round_up_to(FLASH_PAGE_SIZE, sizeof(config_t));
alignas(uint32_t) static uint8_t buffer[config_count_rounded];

static void flash_write_callback(void* param)
{
    const auto* p = static_cast<struct flash_op_params*>(param);
    const auto erase_counts = round_up_to(FLASH_SECTOR_SIZE, p->count);
    flash_range_erase(p->offset, erase_counts);
    flash_range_program(p->offset, p->data, p->count);
}

int flash_write_safe(const uint32_t flash_offs, const uint8_t* data, const size_t count)
{
    memcpy(buffer, data, count);
    memset(buffer + count, 0xFF, config_count_rounded - count);

    flash_op_params p = { flash_offs, buffer, config_count_rounded };
    return flash_safe_execute(flash_write_callback, &p, 100);
}

void config_update()
{
    switch (config.version)
    {
    case 0:
        config = {
            .magic = magic,
            .version = 1,
            .reverse_tt = false,
            .tt_effect = TurntableMode::Spin,
            .tt_deadzone = 4,
            .bar_effect = BarMode::KeySpectrumP1,
            .disable_leds = false,
            .tt_static_hsv = DEFAULT_COLOUR,
            .tt_spin_hsv = DEFAULT_COLOUR,
            .tt_shift_hsv = DEFAULT_COLOUR_NO_HUE,
            .tt_rainbow_static_hsv = DEFAULT_COLOUR_NO_HUE,
            .tt_rainbow_react_hsv = DEFAULT_COLOUR_NO_HUE,
            .tt_rainbow_spin_hsv = DEFAULT_COLOUR_NO_HUE,
            .tt_react_hsv = DEFAULT_COLOUR,
            .tt_breathing_hsv = DEFAULT_COLOUR,
            .tt_ratio = 2,
            .controller_type = ControllerType::IIDX,
            .iidx_input_mode = InputMode::Joystick,
            .sdvx_input_mode = InputMode::Joystick,
            .tt_sustain_ms = 133,
            .iidx_keys = DEFAULT_IIDX_KEYS,
            .sdvx_keys = DEFAULT_SDVX_KEYS,
            .iidx_buttons_debounce = 0,
            .iidx_effectors_debounce = 0,
            .sdvx_buttons_debounce = 0,
            .led_refresh = 60,
            .rainbow_spin_speed = 1,
            .tt_leds = 24
        };
    default:
        break;
    }
}

void config_init()
{
    if (config_mm->magic == magic)
    {
        memcpy(&config, config_mm, sizeof(config_t));
    }

    config_update();
    config.save();
}

void config_t::save() const
{
    if (!equals(config_mm))
        flash_write_safe(FLASH_CONFIG_OFFSET, reinterpret_cast<const uint8_t*>(this), sizeof(config_t));
}

std::optional<callback_t> toggle_reverse_tt()
{
    config.reverse_tt = !config.reverse_tt;
    update_tt_transitions();
    // IIDX::RgbManager::Turntable::reverse_tt(reverse_tt);

    config.save();
    return callback_t{};
}

void update_deadzone(const uint8_t deadzone)
{
    // IIDX::RgbManager::Turntable::display_tt_change(CRGB::Green,
    //                                                deadzone,
    //                                                DEADZONE_MAX);

    config.save();
}

constexpr uint8_t DEADZONE_MAX = 6;
constexpr uint8_t DEADZONE_MIN = 1;

std::optional<callback_t> increase_deadzone()
{
    config.tt_deadzone = MIN(config.tt_deadzone + 1, static_cast<uint8_t>(DEADZONE_MAX));
    update_deadzone(config.tt_deadzone);

    return callback_t{};
}

std::optional<callback_t> decrease_deadzone()
{
    config.tt_deadzone = MAX(config.tt_deadzone - 1, static_cast<uint8_t>(DEADZONE_MIN));
    update_deadzone(config.tt_deadzone);

    return callback_t{};
}

void update_ratio(const uint8_t ratio)
{
    // Present TT ratio as TT sensitivity to the user
    // IIDX::RgbManager::Turntable::display_tt_change(CRGB::Red,
    //                                                RATIO_MAX + 1 - ratio,
    //                                                RATIO_MAX);

    config.save();
}

constexpr uint8_t RATIO_MAX = 6;
constexpr uint8_t RATIO_MIN = 1;

std::optional<callback_t> increase_ratio()
{
    config.tt_ratio = MIN(config.tt_ratio + 1, static_cast<uint8_t>(RATIO_MAX));
    update_ratio(config.tt_ratio);

    return callback_t{};
}

std::optional<callback_t> decrease_ratio()
{
    config.tt_ratio = MAX(config.tt_ratio - 1, static_cast<uint8_t>(RATIO_MIN));
    update_ratio(config.tt_ratio);

    return callback_t{};
}

std::optional<callback_t> toggle_disable_leds()
{
    config.disable_leds = !config.disable_leds;

    if (config.disable_leds)
    {
        // FastLED.clear(true);
    }

    config.save();
    return callback_t{};
}
