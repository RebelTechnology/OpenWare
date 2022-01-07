PROJECT = Xibeca
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = $(OPENWARE)/Hardware/xibeca.ld
include $(OPENWARE)/Hardware/owl3.mk

C_SRC = $(wildcard Core/Src/*.c)
CPP_SRC = $(wildcard Core/Src/*.cpp)
C_SRC += $(OPENWARE)/Source/pcm3168a.c
C_SRC += $(OPENWARE)/Source/usbd_audio.c
# C_SRC += $(OPENWARE)/Source/sdram.c
C_SRC += $(OPENWARE)/Source/sdram-as4cXm32.c

C_SRC += $(OPENWARE)/Source/ssd1309.c
CPP_SRC += $(OPENWARE)/Source/ScreenBuffer.cpp
CPP_SRC += $(OPENWARE)/Source/ScreenBufferMono.cpp
CPP_SRC += $(OPENWARE)/Source/Graphics.cpp

# CPP_SRC += $(OPENWARE)/Source/uart.cpp
# CPP_SRC += $(OPENWARE)/Source/MidiStreamReader.cpp

S_SRC = $(BUILDROOT)/Core/Src/startup_stm32h743xx.s

C_SRC += $(BUILDROOT)/USB_DEVICE/App/usb_device.c
C_SRC += $(BUILDROOT)/USB_DEVICE/App/usbd_desc.c
C_SRC += $(BUILDROOT)/USB_DEVICE/Target/usbd_conf.c

# CPP_SRC += $(OPENWARE)/Source/usbh_midi.cpp
# C_SRC += $(BUILDROOT)/USB_HOST/Target/usbh_conf.c
# C_SRC += $(BUILDROOT)/USB_HOST/App/usb_host.c

C_SRC += $(OPENWARE)/Source/flash.c

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
# C_SRC += $(C_SRC_DAC)
C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_USBD)
# C_SRC += $(C_SRC_USBH)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_I2C)
C_SRC += $(C_SRC_QSPI)
C_SRC += $(C_SRC_MDMA)

include $(OPENWARE)/Hardware/h7.mk
