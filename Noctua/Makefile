PROJECT = Noctua
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = noctua.ld
include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += iis3dwb_reg.c
C_SRC += pcm3168a.c
C_SRC += usbd_audio.c
CPP_SRC += uart.cpp
CPP_SRC += usbh_midi.cpp
CPP_SRC += MidiStreamReader.cpp

include $(OPENWARE)/Hardware/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_SAI)
OBJS += $(OBJS_DAC)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_USBH)
OBJS += $(OBJS_DSP)
OBJS += $(OBJS_OS)
OBJS += $(OBJS_UART)
OBJS += $(OBJS_RTC)
OBJS += $(OBJS_I2C)

include $(OPENWARE)/Hardware/f4.mk
