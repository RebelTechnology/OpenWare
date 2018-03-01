PROJECT = OwlBoot
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/mcu/STM32F427ZI.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))

C_SRC += usbd_midi.c
C_SRC += usbd_midi_if.c
C_SRC += sramalloc.c
C_SRC += sysex.c
C_SRC += crc32.c

# CPP_SRC += MidiReader.cpp

# include $(OPENWARE)/mcu/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_SDRAM)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_UART)

include $(OPENWARE)/mcu/f4.mk
