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
constexpr uint32_t dma_transfer_count = 1;

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

    auto data_chan = dma_claim_unused_channel(true);
    auto ctrl_chan = dma_claim_unused_channel(true);

    auto c_data = dma_channel_get_default_config(data_chan);
    channel_config_set_read_increment(&c_data, false);
    channel_config_set_dreq(&c_data, pio_get_dreq(pio, sm, false));
    channel_config_set_chain_to(&c_data, ctrl_chan);

    dma_channel_configure(
        data_chan, &c_data,
        &position,
        &pio->rxf[sm],
        dma_transfer_count,
        false // Wait for ctrl_chan to start it
    );

    auto c_ctrl = dma_channel_get_default_config(ctrl_chan);
    channel_config_set_read_increment(&c_ctrl, false);
    channel_config_set_chain_to(&c_ctrl, data_chan);

    dma_channel_configure(
        ctrl_chan, &c_ctrl,
        &dma_hw->ch[data_chan].al1_transfer_count_trig,
        &dma_transfer_count,
        1,
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
