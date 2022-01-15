PROJECT = BioSignals
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

# LDSCRIPT = STM32F427VGTx_FLASH.ld
include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
C_SRC += $(OPENWARE)/Source/ble_midi.c
CPP_SRC += $(OPENWARE)/Source/usbd_audio.cpp

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_I2S)
C_SRC += $(C_SRC_RTC)

include $(OPENWARE)/Hardware/f4.mk
