MCU = at90usb1286
ARCH = AVR8
BOARD = BOARD_NONE
F_CPU = 16000000
F_USB = $(F_CPU)
OPTIMIZATION = 2
TARGET = beef
BEEF_LED_REFRESH ?= 40
BEEF_TT_RATIO ?= 2
FASTLED_SRC = FastLED/src
SRC = $(wildcard *.c) $(wildcard *.cpp) \
	$(FASTLED_SRC)/colorutils.cpp $(FASTLED_SRC)/FastLED.cpp $(FASTLED_SRC)/lib8tion.cpp \
	$(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH = LUFA
CC_FLAGS = -DUSE_LUFA_CONFIG_HEADER -IConfig/ -include fastled_shim.h \
	-DFASTLED_NO_PINMAP \
	-DBEEF_LED_REFRESH=$(BEEF_LED_REFRESH) -DBEEF_TT_RATIO=$(BEEF_TT_RATIO)
LD_FLAGS =

# Default target
all:

# Include LUFA-specific DMBS extension modules
DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

# Include common DMBS build system modules
DMBS_PATH ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk

.PHONY: dfu
dfu: all
	sudo avrdude -c flip1 -p usb1286 -U flash:w:$(TARGET).hex

.PHONY: isp
isp: all
	sudo avrdude -c stk500v1 -b 19200 -p $(MCU) -P /dev/ttyACM0 -U flash:w:$(TARGET).hex

.PHONY: fuse
fuse:
	sudo avrdude -c stk500v1 -b 19200 -p $(MCU) -P /dev/ttyACM0 -U lfuse:w:0xde:m -U hfuse:w:0x99:m -U efuse:w:0xf3:m
