PROJECT = Wizard
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
C_SRC += $(OPENWARE)/Source/sdram.c
C_SRC += $(OPENWARE)/Source/cs4272.c

include $(OPENWARE)/Hardware/sources.mk

ifndef PLATFORM
PLATFORM = Wizard
endif

ifeq ($(PLATFORM),EuroWizard)
  CPPFLAGS += -DOWL_EUROWIZARD
else ifeq ($(PLATFORM),Wizard)
  # CPPFLAGS += -DOWL_WIZARD
else
  $(error Invalid PLATFORM=$(PLATFORM))   
endif

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

include $(OPENWARE)/Hardware/f4.mk
