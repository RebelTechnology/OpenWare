PROJECT = MagusEncoder
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/mcu/STM32F051K4.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
S_SRC = startup_stm32f051x8.s

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

include $(OPENWARE)/mcu/f0.mk
