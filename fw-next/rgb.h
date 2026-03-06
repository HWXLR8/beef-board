#pragma once

#include "fastled.h"
#include "hid.h"
#include "patterns.h"

constexpr uint8_t BAR_LEDS = 16;

struct hsv_t
{
    uint8_t h = 0, s = 0, v = 0;

    friend bool operator==(const hsv_t &lhs, const hsv_t &rhs)
    {
        return lhs.h == rhs.h && lhs.s == rhs.s && lhs.v == rhs.v;
    }
};

struct __attribute__((packed)) rgb_t
{
    uint8_t r = 0, g = 0, b = 0;

    explicit operator uint32_t() const
    {
        auto grb = static_cast<uint32_t>(r) << 8 | static_cast<uint32_t>(g) << 16 | static_cast<uint32_t>(b);
        return grb << 8u;
    }

    rgb_t& operator=(const hsv_t &rhs)
    {
        hsv2rgb_rainbow(rhs, *this);
        return *this;
    }
};

template <typename Iterator>
void fill_solid(Iterator begin, Iterator end, uint8_t r, uint8_t g, uint8_t b)
{
    for (auto it = begin; it != end; ++it)
    {
        it->r = r;
        it->g = g;
        it->b = b;
    }
}

template <typename Iterator>
void fill_solid(Iterator begin, Iterator end, const rgb_t &other)
{
    for (auto it = begin; it != end; ++it)
    {
        it->r = other.r;
        it->g = other.g;
        it->b = other.b;
    }
}

template <typename Iterator>
void fill_solid(Iterator begin, Iterator end, const hsv_t &hsv)
{
    rgb_t rgb{};
    hsv2rgb_spectrum(hsv, rgb);
    fill_solid(begin, end, rgb);
}

template <typename Iterator>
void breathing(BreathingPattern &breathing_pattern,
               Iterator begin, Iterator end, const hsv_t &hsv)
{
    const auto v = breathing_pattern.update();
    fill_solid(begin, end, hsv_t{ hsv.h, hsv.s, v });
}

// Share same lighting state between different lights for HID standby animation
inline BreathingPattern hid_standby;

template <typename Iterator>
void hid(Iterator begin, Iterator end, const rgb_t &lights)
{
    if (!hid_expiry_timer.is_active())
    {
        breathing(hid_standby, begin, end, hsv_t{});
        return;
    }
    fill_solid(begin, end, lights);
}

template <typename Iterator>
void set_leds_off(Iterator begin, Iterator end)
{
    fill_solid(begin, end, rgb_t{});
}

uint tt_anim_normalisation();
