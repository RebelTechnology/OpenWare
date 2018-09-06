PROJECT = OwlBoot
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = STM32F427ZI-boot.ld
include $(OPENWARE)/mcu/STM32F427ZI.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))

C_SRC += usbd_midi.c
C_SRC += usbd_midi_if.c
C_SRC += sdram.c
C_SRC += sysex.c
C_SRC += crc32.c
C_SRC += eepromcontrol.c
# S_SRC = startup_openboot.s

# CPP_SRC += MidiReader.cpp
CPP_SRC += MidiController.cpp

# include $(OPENWARE)/mcu/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_SDRAM)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_UART)

include $(OPENWARE)/mcu/f4.mk

flash:
	/home/mars/bin/st-flash write $(BIN) 0x08000000
