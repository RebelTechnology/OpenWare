PROJECT = Magus3
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl3.mk

C_SRC = $(filter-out Src/test.c, $(wildcard Src/*.c))
CPP_SRC = $(wildcard Src/*.cpp)
C_SRC += $(OPENWARE)/Source/sdram.c
C_SRC += $(OPENWARE)/Source/cs4272.c
C_SRC += $(OPENWARE)/Source/ssd1309.c
C_SRC += $(OPENWARE)/Source/HAL_MAX11300.c
C_SRC += $(OPENWARE)/Source/HAL_TLC5946.c
C_SRC += $(OPENWARE)/Source/HAL_Encoders.c
CPP_SRC += $(OPENWARE)/Source/uart.cpp
CPP_SRC += $(OPENWARE)/Source/ScreenBuffer.cpp
CPP_SRC += $(OPENWARE)/Source/ScreenBufferMono.cpp
CPP_SRC += $(OPENWARE)/Source/Graphics.cpp
CPP_SRC += $(OPENWARE)/Source/MidiStreamReader.cpp

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
C_SRC += $(C_SRC_DAC)
C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_USBD_AUDIO)
C_SRC += $(C_SRC_USBH)
C_SRC += $(C_SRC_USBH_MIDI)

include $(OPENWARE)/Hardware/h7.mk
