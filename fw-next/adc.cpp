#include "adc.h"
#include "hardware/adc.h"
#include "hardware/structs/adc.h"

#include "hardware/dma.h"
#include "hardware/structs/dma.h"

volatile uint8_t adc_buf[analog_count];

void adc_dma_init()
{
    adc_set_round_robin((1 << 1) | (1 << 0));
    adc_select_input(0);
    adc_fifo_setup(
        true,   // Write each completed conversion to the sample FIFO
        true,   // Enable DMA data request (DREQ)
        1,      // DREQ (and IRQ) asserted when at least 1 sample present
        false,
        true
    );

    // Divisor of 0 -> full speed. Free-running capture with the divider is
    // equivalent to pressing the ADC_CS_START_ONCE button once per `div + 1`
    // cycles (div not necessarily an integer). Each conversion takes 96
    // cycles, so in general you want a divider of 0 (hold down the button
    // continuously) or > 95 (take samples less frequently than 96 cycle
    // intervals). This is all timed by the 48 MHz ADC clock.
    adc_set_clkdiv(0);

    auto chan = dma_claim_unused_channel(true);
    auto cfg = dma_channel_get_default_config(chan);

    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_8);
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_ring(&cfg, true, 1); // 2^1 = 2 bytes (adc_dma_transfer_count)

    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&cfg, DREQ_ADC);

    dma_channel_configure(
        chan, &cfg,
        adc_buf,
        &adc_hw->fifo,
        dma_encode_endless_transfer_count(),
        true
    );

    adc_run(true);
}
