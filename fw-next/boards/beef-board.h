#ifndef _BOARDS_BEEF_BOARD_H
#define _BOARDS_BEEF_BOARD_H

pico_board_cmake_set(PICO_PLATFORM, rp2350)

// For board detection
#define BEEF_BOARD

// --- RP2350 VARIANT ---
#define PICO_RP2350A 0
#define PICO_RP2350 1 // Needed for FastLED

// --- FLASH ---

#define PICO_BOOT_STAGE2_CHOOSE_W25Q080 1

#ifndef PICO_FLASH_SPI_CLKDIV
#define PICO_FLASH_SPI_CLKDIV 1
#endif

pico_board_cmake_set_default(PICO_FLASH_SIZE_BYTES, (16 * 1024 * 1024))
#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (128 * 1024 * 1024)
#endif

pico_board_cmake_set_default(PICO_RP2350_A2_SUPPORTED, 1)
#ifndef PICO_RP2350_A2_SUPPORTED
#define PICO_RP2350_A2_SUPPORTED 1
#endif

// no PICO_VBUS_PIN
// no PICO_VSYS_PIN

#endif
