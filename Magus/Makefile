PROJECT = Magus
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(filter-out test.c,$(notdir $(wildcard Src/*.c)))
# C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += sdram.c
C_SRC += cs4272.c
C_SRC += usbh_midi.c
C_SRC += ssd1309.c
C_SRC += HAL_MAX11300.c
C_SRC += HAL_TLC5946.c
C_SRC += HAL_Encoders.c
C_SRC += usbd_audio.c
CPP_SRC += ScreenBuffer.cpp ScreenBufferMono.cpp Graphics.cpp
CPP_SRC += MidiStreamReader.cpp

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
OBJS += $(OBJS_UART)

include $(OPENWARE)/Hardware/f4.mk
