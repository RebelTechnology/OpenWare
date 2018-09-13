PROJECT = Prism
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/STM32F745VG.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += cs4272.c seps114a.c
CPP_SRC += ScreenBuffer.cpp ScreenBufferColour.cpp Graphics.cpp

include $(OPENWARE)/Hardware/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(DRIVERS)/Src/stm32f7xx_hal_qspi.o

include $(OPENWARE)/Hardware/f7.mk
