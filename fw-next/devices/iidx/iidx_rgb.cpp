#include "iidx_rgb.h"

#include "beef.h"
#include "config.h"
#include "iidx_combo.h"
#include "rgb.h"
#include "ws2812.h"

constexpr auto SPIN_TIMER = 50;
constexpr auto FAST_SPIN_TIMER = 25;
constexpr auto REACT_TIMER = 500;
constexpr auto BREATHING_TIMER = 3000;

namespace IIDX::RgbManager
{
    namespace Turntable
    {
        // Add a second-long rest period
        BreathingPattern breathing_pattern(BREATHING_TIMER);

        SpinPattern* spin_pattern;
        // Render two spinning LEDs
        void spin(const hsv_t &hsv, const int8_t tt_report)
        {
            if (spin_pattern->update(tt_report))
            {
                set_leds_off(tt_leds.begin(), tt_leds.end());

                const auto colour = hsv_t{ hsv.h, 255, 255 };
                const auto spin_counter = spin_pattern->get();
                tt_leds[spin_counter] = colour;
                tt_leds[spin_counter + (tt_leds.size() / 2)] = colour;
            }
        }

        Ticker colour_shift_ticker(100);

        void colour_shift(const hsv_t &hsv)
        {
            static uint8_t h = 128;

            h += colour_shift_ticker.get_ticks();
            fill_solid(tt_leds.begin(), tt_leds.end(), hsv_t{ h, hsv.s, hsv.v });
        }

        void render_rainbow(const hsv_t &hsv, const uint8_t pos)
        {
            fill_rainbow_circular(tt_leds, -pos, hsv.s, hsv.v);
        }

        void render_rainbow_react(const hsv_t &hsv,
                                  const int8_t tt_report)
        {
            static uint8_t pos = 0;
            pos += tt_report * config.rainbow_spin_speed;
            render_rainbow(hsv, pos);
        }

        SpinPattern rainbow_spin_pattern(3, 2);

        void render_rainbow_spin(const hsv_t &hsv,
                                 const int8_t tt_report)
        {
            if (rainbow_spin_pattern.update(-tt_report))
            {
                const uint8_t pos = rainbow_spin_pattern.get() * config.rainbow_spin_speed;
                render_rainbow(hsv, pos);
            }
        }

        timer_t scr_timer;

        void react(const int8_t tt_report, const hsv_t &hsv)
        {
            static uint8_t h = hsv.h;

            switch (tt_report)
            {
            case 1:
                h = hsv.h;
                scr_timer.arm(REACT_TIMER);
                break;
            case -1:
                h = hsv.h + 128;
                scr_timer.arm(REACT_TIMER);
                break;
            default:
                break;
            }

            const uint8_t v = scr_timer.is_active() ? 255 : 64;

            fill_solid(tt_leds.begin(), tt_leds.end(), hsv_t{ h, hsv.s, v });
        }

        // Match tt_report with physical turntable movement
        // -1 is clockwise, +1 is counter-clockwise
        int8_t normalise_tt_report(const int8_t tt_report)
        {
            return config.reverse_tt ? tt_report : -tt_report;
        }

        void update(int8_t tt1_report, const rgb_t &lights)
        {
            // Ignore turntable effect if notifying a setting change
            if (combo_timer.is_active())
            {
                return;
            }

            switch (config.tt_effect)
            {
            case TurntableMode::Static:
                fill_solid(tt_leds.begin(), tt_leds.end(), config.tt_static_hsv);
                break;
            case TurntableMode::Spin:
                tt1_report = normalise_tt_report(tt1_report);
                spin(config.tt_spin_hsv, tt1_report);
                break;
            case TurntableMode::Shift:
                colour_shift(config.tt_shift_hsv);
                break;
            case TurntableMode::RainbowStatic:
                render_rainbow(config.tt_rainbow_static_hsv, 0);
                break;
            case TurntableMode::RainbowReact:
                tt1_report = normalise_tt_report(tt1_report);
                render_rainbow_react(config.tt_rainbow_react_hsv,
                                     tt1_report);
                break;
            case TurntableMode::RainbowSpin:
                render_rainbow_spin(config.tt_rainbow_spin_hsv,
                                    tt1_report);
                break;
            case TurntableMode::React:
                react(tt1_report, config.tt_react_hsv);
                break;
            case TurntableMode::Breathing:
                breathing(breathing_pattern,
                          tt_leds.begin(), tt_leds.end(),
                          config.tt_breathing_hsv);
                break;
            case TurntableMode::HID:
                hid(tt_leds.begin(), tt_leds.end(), lights);
                break;
            case TurntableMode::Disable:
                set_leds_off(tt_leds.begin(), tt_leds.end());
                break;
            default:
                break;
            }
        }
    }

    namespace Bar
    {
        enum PlayerSide
        {
            P1,
            P2
        };

        void update(const rgb_t &lights)
        {
            switch (config.bar_effect)
            {
            case BarMode::KeySpectrumP1:
                break;
            case BarMode::KeySpectrumP2:
                break;
            case BarMode::HID:
                hid(bar_leds.begin(), bar_leds.end(), lights);
                break;
            case BarMode::TapeLedP1:
                break;
            case BarMode::TapeLedP2:
                break;
            case BarMode::Disable:
                set_leds_off(bar_leds.begin(), bar_leds.end());
                break;
            default:
                break;
            }
        }
    }

    void init()
    {
        Turntable::spin_pattern = new SpinPattern(SPIN_TIMER * tt_anim_normalisation(),
                                                  FAST_SPIN_TIMER * tt_anim_normalisation(),
                                                  config.tt_leds / 2);
    }

    void update(const int8_t tt1_report,
                const hid_lights_t &led_state_from_hid_report)
    {
        if (config.disable_leds)
            return;

        Turntable::update(tt1_report,
                          led_state_from_hid_report.tt_lights);

        Bar::update(led_state_from_hid_report.bar_lights);
    }
}
