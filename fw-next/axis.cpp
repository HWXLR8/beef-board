#include "axis.h"

#include "adc.h"
#include "config.h"
#include "quadrature_encoder.pio.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

AnalogAxis::AnalogAxis(const uint8_t pin) : pin(pin)
{
    adc_gpio_init(pin);
}

void AnalogAxis::poll()
{
    position = adc_buf[pin - ADC_BASE_PIN];
}

uint8_t AnalogAxis::get() const
{
    return position;
}

static PIO pio;

QeAxis::QeAxis(uint8_t a_pin)
{
    uint sm;
    uint offset;
    if (pio == nullptr)
    {
        pio_claim_free_sm_and_add_program_for_gpio_range(&quadrature_encoder_program, &pio, &sm, &offset,
                                                         a_pin, 2, true);
    }
    else
    {
        sm = pio_claim_unused_sm(pio, true);
        if (sm == PICO_ERROR_GENERIC)
        {
            // Used up all SMs for PIO, try to get new one
            pio_claim_free_sm_and_add_program_for_gpio_range(&quadrature_encoder_program, &pio, &sm, &offset,
                                                             a_pin, 2, true);
        }
    }

    quadrature_encoder_program_init(pio, sm, a_pin, 0);

    auto chan = dma_claim_unused_channel(true);
    auto cfg = dma_channel_get_default_config(chan);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_dreq(&cfg, pio_get_dreq(pio, sm, false));

    dma_channel_configure(
        chan, &cfg,
        &position,
        &pio->rxf[sm],
        dma_encode_endless_transfer_count(),
        true
    );
}

void QeAxis::poll()
{
    // DMA will automatically update position data
}

uint8_t QeAxis::get() const
{
    const auto max = 256 * config.tt_ratio;
    auto pos = position % max;
    if (!config.reverse_tt) pos = max - pos;
    return pos / config.tt_ratio;
}
