# Copyright 2021 Dean Camera (dean [at] fourwalledcubicle [dot] com)
# Copyright 2023 supervaka, HWXLR8

MCU = at90usb1286
ARCH = AVR8
BOARD = BOARD_NONE
F_CPU = 16000000
F_USB = $(F_CPU)
OPTIMIZATION = s
TARGET = beef
SRC = $(TARGET).c Descriptors.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH = LUFA
CC_FLAGS = -DUSE_LUFA_CONFIG_HEADER -IConfig/
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

.PHONY: flash
flash:
	sudo avrdude -c flip1 -p usb1286 -U flash:w:$(TARGET).hex
