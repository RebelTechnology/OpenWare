PROJECT = MagusEncoder
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..
OPENOCD ?= openocd -f openocd.cfg

include $(OPENWARE)/Hardware/STM32F051K4.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)

include $(OPENWARE)/Hardware/f0.mk
