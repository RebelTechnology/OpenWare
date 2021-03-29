PROJECT = Mussol
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/mussol.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
C_SRC += $(OPENWARE)/Source/usbd_audio.c
C_SRC += $(OPENWARE)/Source/usbh_midi.c
C_SRC += $(OPENWARE)/Source/HAL_MAX11300.c
# C_SRC += $(OPENWARE)/Source/ssd1309.c
# CPP_SRC += $(OPENWARE)/Source/ScreenBuffer.cpp
# CPP_SRC += $(OPENWARE)/Source/ScreenBufferMono.cpp
# CPP_SRC += $(OPENWARE)/Source/Graphics.cpp
CPP_SRC += $(OPENWARE)/Source/MidiStreamReader.cpp

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_USBH)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_RTC)
C_SRC += $(C_SRC_I2C)

include $(OPENWARE)/Hardware/f4.mk
