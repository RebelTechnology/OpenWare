PROJECT = MagusEncoder
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..
OPENOCD ?= openocd -f openocd.cfg

include $(OPENWARE)/Hardware/STM32F051K4.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

include $(OPENWARE)/Hardware/f0.mk
