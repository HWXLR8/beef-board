#include "fastled.h"

#include "rgb.h"

uint8_t scale8(uint8_t i, uint8_t scale)
{
    return ((uint16_t)i * (1 + (uint16_t)scale)) >> 8;
}

uint8_t quadwave8(uint8_t i)
{
    uint8_t j = i;
    if (j & 0x80)
    {
        j = 255 - j;
    }
    uint8_t jj = scale8(j, j);
    uint8_t jj2 = jj << 1;
    if (i & 0x80)
    {
        jj2 = 255 - jj2;
    }
    return jj2;
}

/// Divide the color wheel into four sections, 64 elements each
#define HSV_SECTION_4 (0x40)

void hsv2rgb_raw(const hsv_t &hsv, rgb_t &rgb)
{
    // Convert hue, saturation and brightness ( HSV/HSB ) to RGB
    // "Dimming" is used on saturation and brightness to make
    // the output more visually linear.

    // Apply dimming curves
    uint8_t value = hsv.v;
    uint8_t saturation = hsv.s;

    // The brightness floor is minimum number that all of
    // R, G, and B will be set to.
    uint8_t invsat = 255 - saturation;
    uint8_t brightness_floor = (value * invsat) / 256;

    // The color amplitude is the maximum amount of R, G, and B
    // that will be added on top of the brightness_floor to
    // create the specific hue desired.
    uint8_t color_amplitude = value - brightness_floor;

    // Figure out which section of the hue wheel we're in,
    // and how far offset we are withing that section
    uint8_t section = hsv.h / HSV_SECTION_4; // 0..2
    uint8_t offset = hsv.h % HSV_SECTION_4;  // 0..63

    uint8_t rampup = offset; // 0..63
    uint8_t rampdown = (HSV_SECTION_4 - 1) - offset; // 63..0

    // We now scale rampup and rampdown to a 0-255 range -- at least
    // in theory, but here's where architecture-specific decsions
    // come in to play:
    // To scale them up to 0-255, we'd want to multiply by 4.
    // But in the very next step, we multiply the ramps by other
    // values and then divide the resulting product by 256.
    // So which is faster?
    //   ((ramp * 4) * othervalue) / 256
    // or
    //   ((ramp    ) * othervalue) /  64
    // It depends on your processor architecture.
    // On 8-bit AVR, the "/ 256" is just a one-cycle register move,
    // but the "/ 64" might be a multicycle shift process. So on AVR
    // it's faster do multiply the ramp values by four, and then
    // divide by 256.
    // On ARM, the "/ 256" and "/ 64" are one cycle each, so it's
    // faster to NOT multiply the ramp values by four, and just to
    // divide the resulting product by 64 (instead of 256).
    // Moral of the story: trust your profiler, not your insticts.

    // Since there's an AVR assembly version elsewhere, we'll
    // assume what we're on an architecture where any number of
    // bit shifts has roughly the same cost, and we'll remove the
    // redundant math at the source level:

    //  // scale up to 255 range
    //  //rampup *= 4; // 0..252
    //  //rampdown *= 4; // 0..252

    // compute color-amplitude-scaled-down versions of rampup and rampdown
    uint8_t rampup_amp_adj = (rampup * color_amplitude) / (256 / 4);
    uint8_t rampdown_amp_adj = (rampdown * color_amplitude) / (256 / 4);

    // add brightness_floor offset to everything
    uint8_t rampup_adj_with_floor = rampup_amp_adj + brightness_floor;
    uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;


    if (section)
    {
        if (section == 1)
        {
            // section 1: 0x40..0x7F
            rgb.r = brightness_floor;
            rgb.g = rampdown_adj_with_floor;
            rgb.b = rampup_adj_with_floor;
        }
        else
        {
            // section 2; 0x80..0xBF
            rgb.r = rampup_adj_with_floor;
            rgb.g = brightness_floor;
            rgb.b = rampdown_adj_with_floor;
        }
    }
    else
    {
        // section 0: 0x00..0x3F
        rgb.r = rampdown_adj_with_floor;
        rgb.g = rampup_adj_with_floor;
        rgb.b = brightness_floor;
    }
}

void hsv2rgb_spectrum(const hsv_t &hsv, rgb_t &rgb)
{
    hsv_t hsv2(hsv);
    hsv2.h = scale8(hsv2.h, 191);
    hsv2rgb_raw(hsv2, rgb);
}
