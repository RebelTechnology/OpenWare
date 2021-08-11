PROJECT = Noctua
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = noctua.ld
include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
C_SRC += $(OPENWARE)/Source/iis3dwb_reg.c
C_SRC += $(OPENWARE)/Source/pcm3168a.c
C_SRC += $(OPENWARE)/Source/usbd_audio.c
CPP_SRC += $(OPENWARE)/Source/uart.cpp
CPP_SRC += $(OPENWARE)/Source/uart_midi.cpp
CPP_SRC += $(OPENWARE)/Source/usbh_midi.cpp
CPP_SRC += $(OPENWARE)/Source/MidiStreamReader.cpp

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
C_SRC += $(C_SRC_DAC)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_USBH)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_RTC)
C_SRC += $(C_SRC_I2C)

include $(OPENWARE)/Hardware/f4.mk
