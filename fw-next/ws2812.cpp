#include "ws2812.h"

#include <array>

#include "beef.h"
#include "config.h"
#include "limiter.h"
#include "pins.h"
#include "ws2812.pio.h"
#include "bsp/board_api.h"
#include "hardware/dma.h"
#include "hardware/pio.h"

std::vector<rgb_t> bar_leds = std::vector<rgb_t>(BAR_LEDS);
std::array<uint32_t, BAR_LEDS> bar_leds_dma;
std::vector<rgb_t> tt_leds;
std::vector<uint32_t> tt_leds_dma;

int dma_bar;
int dma_tt;

void ws2812_init()
{
    tt_leds = std::vector<rgb_t>(config.tt_leds);
    tt_leds_dma = std::vector<uint32_t>(config.tt_leds);

    // This will find a free pio and state machine for our program and load it for us
    // We use pio_claim_free_sm_and_add_program_for_gpio_range (for_gpio_range variant)
    // so we will get a PIO instance suitable for addressing gpios >= 32 if needed and supported by the hardware
    PIO pio;
    uint sm_bar;
    uint offset;
    pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm_bar, &offset,
                                                     BAR_RGB_GPIO, 1, true);
    ws2812_program_init(pio, sm_bar, offset, BAR_RGB_GPIO, 800000, false);

    dma_bar = dma_claim_unused_channel(true);
    auto dma_config_bar = dma_channel_get_default_config(dma_bar);
    channel_config_set_dreq(&dma_config_bar, pio_get_dreq(pio, sm_bar, true));
    dma_channel_configure(dma_bar, &dma_config_bar,
                          &pio->txf[sm_bar],
                          bar_leds_dma.data(),
                          bar_leds_dma.size(),
                          false
    );

    const uint sm_tt = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm_tt, offset, TT_RGB_GPIO, 800000, false);

    dma_tt = dma_claim_unused_channel(true);
    auto dma_config_tt = dma_channel_get_default_config(dma_tt);
    channel_config_set_dreq(&dma_config_tt, pio_get_dreq(pio, sm_tt, true));
    dma_channel_configure(dma_tt, &dma_config_tt,
                          &pio->txf[sm_tt],
                          tt_leds_dma.data(),
                          tt_leds_dma.size(),
                          false
    );
}

void ws2812_show()
{
    if (!dma_channel_is_busy(dma_bar))
    {
        for (int i = 0; i < BAR_LEDS; i++)
        {
            bar_leds_dma[i] = static_cast<uint32_t>(bar_leds[i]);
        }
        dma_channel_set_read_addr(dma_bar, bar_leds_dma.data(), true);
    }

    if (!dma_channel_is_busy(dma_tt))
    {
        for (int i = 0; i < tt_leds.size(); i++)
        {
            tt_leds_dma[i] = static_cast<uint32_t>(tt_leds[i]);
        }
        dma_channel_set_read_addr(dma_tt, tt_leds_dma.data(), true);
    }
}

Limiter limiter(1000000 / 400);

bool ready_to_show()
{
    return limiter.ready();
}
