BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl1.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)
C_SRC += $(OPENWARE)/Source/wm8731.c
CPP_SRC += $(OPENWARE)/Source/usbd_audio.cpp

ifndef PLATFORM
PLATFORM = Pedal
endif

ifeq ($(PLATFORM),Pedal)
  PROJECT = OwlPedal
  CPPFLAGS += -DOWL_PEDAL
else ifeq ($(PLATFORM),Modular)
  PROJECT = OwlModular
  CPPFLAGS += -DOWL_MODULAR
else ifeq ($(PLATFORM),Rack)
  PROJECT = OwlRack
  CPPFLAGS += -DOWL_RACK
  CPP_SRC += DigitalBusHandler.cpp DigitalBusReader.cpp DigitalBusStreamReader.cpp bus.cpp
else
  $(error Invalid PLATFORM=$(PLATFORM))
endif

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_I2S)
C_SRC += $(C_SRC_SRAM)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)

include $(OPENWARE)/Hardware/f4.mk

DFUCMD = dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -R
dfu: bin
	@echo Flashing OWL $(PLATFORM) $(CONFIG) firmware
	$(DFUCMD) -s 0x8008000:leave -D $(BIN)
