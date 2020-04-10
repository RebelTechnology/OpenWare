PROJECT = OwlPedal
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

include $(OPENWARE)/Hardware/owl1.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))
C_SRC += wm8731.c
C_SRC += usbd_midi.c
C_SRC += usbd_midi_if.c

ifndef PLATFORM
PLATFORM = Pedal
endif

ifeq ($(PLATFORM),Pedal)
  CPPFLAGS += -DOWL_PEDAL
else ifeq ($(PLATFORM),Modular)
  CPPFLAGS += -DOWL_MODULAR
else ifeq ($(PLATFORM),Rack)
  CPPFLAGS += -IOWL_PEDAL -DOWL_RACK
  CPP_SRC += DigitalBusHandler.cpp DigitalBusReader.cpp DigitalBusStreamReader.cpp bus.cpp
endif

include $(OPENWARE)/Hardware/sources.mk

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_I2S)
OBJS += $(OBJS_SRAM)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_DSP)
OBJS += $(OBJS_OS)
OBJS += $(OBJS_UART)

include $(OPENWARE)/Hardware/f4.mk

DFUCMD = dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -R
dfu: bin
	@echo Flashing OWL $(PLATFORM) $(CONFIG) firmware
	$(DFUCMD) -s 0x8008000:leave -D $(BIN)
