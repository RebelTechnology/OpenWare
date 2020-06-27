PROJECT = Witch
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += sdram.c
C_SRC += cs4272.c
C_SRC += usbd_audio.c
CPP_SRC += usbh_midi.cpp

include $(OPENWARE)/Hardware/sources.mk

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
OBJS += $(OBJS_RTC)

include $(OPENWARE)/Hardware/f4.mk
