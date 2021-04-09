PROJECT = Expander
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..
OPENOCD ?= openocd -f openocd.cfg

include $(OPENWARE)/Hardware/STM32F100CB.mk

C_SRC = $(wildcard Src/*.c)
#C_SRC = $(filter-out Src/test.c, $(wildcard Src/*.c))
C_SRC += $(wildcard Source/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
CPP_SRC += $(wildcard Source/*.cpp)
C_SRC += $(OPENWARE)/Source/HAL_MAX11300.c
C_SRC += $(OPENWARE)/Source/HAL_TLC5946.c

include $(OPENWARE)/Hardware/f1.mk
