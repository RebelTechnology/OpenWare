PROJECT = Lich3
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl3.mk

C_SRC = $(wildcard Core/Src/*.c)
CPP_SRC = $(wildcard Core/Src/*.cpp)
C_SRC += $(OPENWARE)/Source/sdram.c
C_SRC += $(OPENWARE)/Source/cs4272.c
CPP_SRC += $(OPENWARE)/Source/uart.cpp
CPP_SRC += $(OPENWARE)/Source/bus.cpp
CPP_SRC += $(OPENWARE)/Source/DigitalBusHandler.cpp
CPP_SRC += $(OPENWARE)/Source/DigitalBusReader.cpp
CPP_SRC += $(OPENWARE)/Source/DigitalBusStreamReader.cpp
# CPP_SRC += $(OPENWARE)/Source/MidiStreamReader.cpp

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
C_SRC += $(C_SRC_DAC)
C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_RTC)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_USBD_AUDIO)
C_SRC += $(C_SRC_USBH)
C_SRC += $(C_SRC_USBH_MIDI)

include $(OPENWARE)/Hardware/h7.mk
