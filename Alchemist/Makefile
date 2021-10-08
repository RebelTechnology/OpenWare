PROJECT = Alchemist
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
C_SRC += $(OPENWARE)/Source/sdram.c
C_SRC += $(OPENWARE)/Source/cs4272.c
CPP_SRC += $(OPENWARE)/Source/usbd_audio.cpp

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_RTC)

include $(OPENWARE)/Hardware/f4.mk
