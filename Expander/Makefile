PROJECT = Expander
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..
OPENOCD ?= openocd -f openocd.cfg

include $(OPENWARE)/Hardware/STM32F100CB.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
CPP_SRC += $(OPENWARE)/Source/bus.cpp
CPP_SRC += $(OPENWARE)/Source/DigitalBusReader.cpp
CPP_SRC += $(OPENWARE)/Source/DigitalBusHandler.cpp
CPP_SRC += $(OPENWARE)/Source/DigitalBusStreamReader.cpp
CPP_SRC += $(OPENWARE)/Source/MidiReader.cpp
C_SRC += $(OPENWARE)/Source/errorhandlers.c
C_SRC += $(OPENWARE)/Source/HAL_MAX11300.c
C_SRC += $(OPENWARE)/Source/HAL_TLC5946.c

include $(OPENWARE)/Hardware/f1.mk
