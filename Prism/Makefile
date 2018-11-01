PROJECT = Prism
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl2.mk
# Override LDSCRIPT for no bootloader.
# Edit Src/system_stm32f4xx.c VECT_TAB_OFFSET to enable bootloader
LDSCRIPT = $(OPENWARE)/Hardware/STM32F439ZG.ld

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += cs4272.c seps114a.c
CPP_SRC += ScreenBuffer.cpp ScreenBufferColour.cpp Graphics.cpp

CPP_SRC += bus.cpp
CPP_SRC += DigitalBusHandler.cpp DigitalBusReader.cpp DigitalBusStreamReader.cpp 

include $(OPENWARE)/Hardware/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_SAI)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_DSP)
OBJS += $(OBJS_OS)
OBJS += $(OBJS_UART)

include $(OPENWARE)/Hardware/f4.mk
