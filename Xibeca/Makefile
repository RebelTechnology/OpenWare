PROJECT = Xibeca
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/xibeca.mk

C_SRC = $(wildcard Core/Src/*.c)
CPP_SRC = $(wildcard Core/Src/*.cpp)
C_SRC += $(OPENWARE)/Source/pcm3168a.c
C_SRC += $(OPENWARE)/Source/sdram-as4cXm32.c

C_SRC += $(OPENWARE)/Source/ssd1309.c
CPP_SRC += $(OPENWARE)/Source/ScreenBuffer.cpp
CPP_SRC += $(OPENWARE)/Source/ScreenBufferMono.cpp
CPP_SRC += $(OPENWARE)/Source/Graphics.cpp

# CPP_SRC += $(OPENWARE)/Source/uart.cpp
# CPP_SRC += $(OPENWARE)/Source/MidiStreamReader.cpp

C_SRC += $(OPENWARE)/Source/flash.c

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
# C_SRC += $(C_SRC_DAC)
C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_I2C)
C_SRC += $(C_SRC_QSPI)
C_SRC += $(C_SRC_MDMA)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_USBD_AUDIO)
C_SRC += $(C_SRC_USBH)
C_SRC += $(C_SRC_USBH_MIDI)

include $(OPENWARE)/Hardware/h7.mk
