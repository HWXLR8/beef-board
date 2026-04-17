#include "adc.h"
#include "hardware/adc.h"
#include "hardware/structs/adc.h"

#include "hardware/dma.h"
#include "hardware/structs/dma.h"

volatile uint8_t adc_buf[adc_dma_transfer_count];

void adc_dma_init()
{
    adc_set_round_robin((1 << 1) | (1 << 0));
    adc_select_input(0);
    adc_fifo_setup(
        true,                   // Write each completed conversion to the sample FIFO
        true,                   // Enable DMA data request (DREQ)
        adc_dma_transfer_count, // DREQ (and IRQ) asserted when at least 2 samples present
        false,                  // We won't see the ERR bit because of 8 bit reads; disable.
        true                    // Shift each sample to 8 bits when pushing to FIFO
    );

    // Divisor of 0 -> full speed. Free-running capture with the divider is
    // equivalent to pressing the ADC_CS_START_ONCE button once per `div + 1`
    // cycles (div not necessarily an integer). Each conversion takes 96
    // cycles, so in general you want a divider of 0 (hold down the button
    // continuously) or > 95 (take samples less frequently than 96 cycle
    // intervals). This is all timed by the 48 MHz ADC clock.
    adc_set_clkdiv(0);

    auto data_chan = dma_claim_unused_channel(true);
    auto ctrl_chan = dma_claim_unused_channel(true);

    auto c_data = dma_channel_get_default_config(data_chan);

    // Reading from constant address, writing to incrementing byte addresses
    channel_config_set_transfer_data_size(&c_data, DMA_SIZE_8);
    channel_config_set_read_increment(&c_data, false);
    channel_config_set_write_increment(&c_data, true);
    channel_config_set_ring(&c_data, true, 1); // 2^1 = 2 bytes (adc_dma_transfer_count)

    // Pace transfers based on availability of ADC samples
    channel_config_set_dreq(&c_data, DREQ_ADC);
    channel_config_set_chain_to(&c_data, ctrl_chan);

    dma_channel_configure(
        data_chan, &c_data,
        adc_buf,
        &adc_hw->fifo,
        adc_dma_transfer_count,
        false // Wait for ctrl_chan to start it
    );

    auto c_ctrl = dma_channel_get_default_config(ctrl_chan);
    channel_config_set_read_increment(&c_ctrl, false);
    channel_config_set_chain_to(&c_ctrl, data_chan);

    dma_channel_configure(
        ctrl_chan, &c_ctrl,
        &dma_hw->ch[data_chan].al1_transfer_count_trig,
        &adc_dma_transfer_count,
        1,
        true
    );

    adc_run(true);
}
