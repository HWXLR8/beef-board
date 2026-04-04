#include <optional>

#include "combo.h"
#include "beef.h"

timer_t combo_lights_timer;
timer_t combo_timer;
bool combo_activated = false;
bool ignore_combo = false;
std::optional<callback_t> config_update_callback;

void process_combos()
{
    reactive_leds = false;
    const auto button_combo = usb->get_button_combo();
    if (button_combo.update_config != nullptr)
    {
        if (ignore_combo)
            return;

        combo_activated = true;
        reactive_leds = true;

        if (button_combo.continuous)
        {
            auto cb = button_combo.update_config();
            if (!cb.has_value())
            {
                // Invalid combo held
                combo_lights_timer.arm(250);
            }
            else
            {
                config_update_callback = cb;
            }
        }
        else
        {
            // arm timer if not already armed
            if (!combo_timer.armed)
                combo_timer.arm(1000);

            if (combo_timer.is_expired(true))
            {
                config_update_callback = button_combo.update_config();
                combo_lights_timer.arm(CONFIG_CHANGE_NOTIFY_TIME);
                ignore_combo = true;
            }
        }

        return;
    }

    if (combo_activated)
    {
        combo_activated = false;
        ignore_combo = false;

        combo_timer.reset();
        combo_lights_timer.reset();
        usb->on_combo_reset();

        if (config_update_callback.value_or(callback_t{}).deferred_save)
            config.save();
    }
}
