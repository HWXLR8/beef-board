#pragma once

#include <optional>

#include "config.h"
#include "timer.h"

struct combo_t
{
    bool continuous;
    std::optional<callback_t> (*update_config)();
};

class ComboProcessor
{
public:
    virtual ~ComboProcessor() = default;

    virtual combo_t get_button_combo() = 0;
    virtual void on_reset() = 0;
};

extern timer_t combo_lights_timer;

void combo_init();
void process_combos();
