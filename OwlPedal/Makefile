PROJECT = OwlPedal
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl1.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += wm8731.c

CPP_SRC += bus.cpp
CPP_SRC += DigitalBusHandler.cpp DigitalBusReader.cpp DigitalBusStreamReader.cpp 

include $(OPENWARE)/Hardware/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_I2S)
OBJS += $(OBJS_SRAM)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_DSP)
OBJS += $(OBJS_OS)
OBJS += $(OBJS_UART)

include $(OPENWARE)/Hardware/f4.mk
