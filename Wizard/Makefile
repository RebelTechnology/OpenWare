PROJECT = Wizard
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = flash.ld
include $(OPENWARE)/mcu/STM32F427ZI.mk

# C_SRC = $(filter-out usbd_audio_if.c $(notdir $(wildcard Src/*.c)))
C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += sdram.c
C_SRC += cs4272.c
C_SRC += usbh_midi.c

include $(OPENWARE)/mcu/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_SAI)
OBJS += $(OBJS_DAC)
OBJS += $(OBJS_SDRAM)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_USBH)
OBJS += $(OBJS_DSP)
OBJS += $(OBJS_OS)
OBJS += $(OBJS_UART)

include $(OPENWARE)/mcu/f4.mk
