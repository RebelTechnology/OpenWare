BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..
DFUUTIL ?= dfu-util

LDSCRIPT = STM32F405ZGT6-boot.ld
include $(OPENWARE)/Hardware/owl1.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)

# C_SRC += usbd_audio.c
C_SRC += $(OPENWARE)/Source/usbd_midi.c
C_SRC += $(OPENWARE)/Source/sysex.c
C_SRC += $(OPENWARE)/Source/crc32.c
C_SRC += $(OPENWARE)/Source/eepromcontrol.c
C_SRC += $(OPENWARE)/Source/message.c
CPP_SRC += $(OPENWARE)/Source/MidiController.cpp
CPP_SRC += $(OPENWARE)/Source/MidiWriter.cpp
CPP_SRC += $(OPENWARE)/Source/VersionToken.cpp

ifndef PLATFORM
PLATFORM = Pedal
endif

PROJECT = MidiBoot-${PLATFORM}

ifeq ($(PLATFORM),Pedal)
  CPPFLAGS += -Iusbd-fs -DOWL_PEDAL
#  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Modular)
  CPPFLAGS += -Iusbd-fs -DOWL_MODULAR
#  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Rack)
  CPPFLAGS += -Iusbd-fs -IOWL_PEDAL -DOWL_RACK
#  vpath %.c usbd-fs
else
  $(error Invalid PLATFORM=$(PLATFORM))
endif

C_SRC += $(C_SRC_SRAM)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_IWDG)

include $(OPENWARE)/Hardware/f4.mk

.PHONY: clean deploy unlock erase upload lock

info:
	$(OPENOCD) -c "init" -c "flash info 0" -c "exit"

unlock:
	$(OPENOCD) -c "init" -c "reset halt" -c "stm32f4x unlock 0" -c "exit"
	$(OPENOCD) -c "init" -c "reset halt" -c "flash protect 0 0 11 off" -c "flash info 0" -c "exit"

erase:
	$(OPENOCD) -c "init" -c "halt" -c "stm32f2x mass_erase 0" -c "flash info 0" -c "exit"

erase-boot: # flash erase_sector num first last
	$(OPENOCD) -c "init" -c "halt" -c "flash erase_sector 0 0 1" -c "exit"

erase-storage: # flash erase_sector num first last
	$(OPENOCD) -c "init" -c "halt" -c "flash erase_sector 0 7 11" -c "exit"

dump: # flash read_bank num filename [offset [length]]
	$(OPENOCD) -c "init" -c "halt" -c "flash read_bank 0 dump.bin" -c "exit"

upload:
	$(OPENOCD) -c "program Build/${PROJECT}.elf verify reset exit"

lock:
	$(OPENOCD) -c "init" -c "halt" -c "flash protect 0 0 1 on" -c "flash info 0" -c "exit"

dfu: Build/${PROJECT}.elf
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000 -D $<
	@echo Uploaded $< to OWL bootsector

sysex:
	FirmwareSender -in "Build/${PROJECT}.bin" -flash `crc32 "Build/${PROJECT}.bin"` -save "Build/${PROJECT}.syx"
