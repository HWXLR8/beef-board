#pragma once

#include <cstdint>

constexpr uint8_t BAR_LEDS = 16;

struct __attribute__((packed)) rgb_t
{
    uint8_t r = 0, g = 0, b = 0;

    explicit operator uint32_t() const
    {
        auto grb = static_cast<uint32_t>(r) << 8 | static_cast<uint32_t>(g) << 16 | static_cast<uint32_t>(b);
        return grb << 8u;
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
