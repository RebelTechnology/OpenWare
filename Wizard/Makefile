PROJECT = Wizard
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
C_SRC += $(OPENWARE)/Source/sdram.c
C_SRC += $(OPENWARE)/Source/cs4272.c
C_SRC += $(OPENWARE)/Source/usb_device.c
C_SRC += $(OPENWARE)/Source/usbd_desc.c
C_SRC += $(OPENWARE)/Source/usbd_conf.c
C_SRC += $(OPENWARE)/Source/usbh_conf.c
CPP_SRC += $(OPENWARE)/Source/usbd_audio.cpp
CPP_SRC += $(OPENWARE)/Source/usbh_midi.cpp

include $(OPENWARE)/Hardware/sources.mk

ifndef PLATFORM
PLATFORM = Wizard
endif

ifeq ($(PLATFORM),EuroWizard)
  CPPFLAGS += -Iusbd-fs -DOWL_EUROWIZARD
  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Wizard)
  CPPFLAGS += -Iusbd-hs
  vpath %.c usbd-hs
endif

C_SRC += $(C_SRC_SAI)
C_SRC += $(C_SRC_DAC)
C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_USBH)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_RTC)

include $(OPENWARE)/Hardware/f4.mk
