BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..
OPENOCD ?= openocd -f $(OPENWARE)/Hardware/openocd_h7.cfg

LDSCRIPT = STM32H743ZGTx-boot.ld
include $(OPENWARE)/Hardware/owl3.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)

# C_SRC += usbd_audio.c
C_SRC += $(OPENWARE)/Source/usbd_midi.c
C_SRC += $(OPENWARE)/Source/sdram.c
C_SRC += $(OPENWARE)/Source/sysex.c
C_SRC += $(OPENWARE)/Source/crc32.c
C_SRC += $(OPENWARE)/Source/eepromcontrol.c
C_SRC += $(OPENWARE)/Source/message.c
CPP_SRC += $(OPENWARE)/Source/MidiWriter.cpp
CPP_SRC += $(OPENWARE)/Source/VersionToken.cpp
# S_SRC = startup_openboot.s

ifndef PLATFORM
PLATFORM = Genius
endif

PROJECT = MidiBoot-$(PLATFORM)

ifeq ($(PLATFORM),Genius)
  CPPFLAGS += -Iusbd-fs -DOWL_GENIUS -DUSE_USBD_FS
  vpath %.c usbd-fs
endif

CPP_SRC += $(OPENWARE)/Source/MidiController.cpp

C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_IWDG)

include $(OPENWARE)/Hardware/h7.mk

.PHONY: clean deploy unlock erase upload lock

info:
	$(OPENOCD) -c "init" -c "flash info 0" -c "exit"

unlock:
	$(OPENOCD) -c "init" -c "reset halt" -c "stm32h7x unlock 0" -c "exit"
	$(OPENOCD) -c "init" -c "reset halt" -c "flash protect 0 0 1 off" -c "flash info 0" -c "exit"

erase:
	$(OPENOCD) -c "init" -c "halt" -c "stm32h7 mass_erase 0" -c "flash info 0" -c "exit"

erase-boot: # flash erase_sector num first last
	$(OPENOCD) -c "init" -c "halt" -c "flash erase_sector 0 0 1" -c "exit"

erase-storage: # flash erase_sector num first last
	$(OPENOCD) -c "init" -c "halt" -c "flash erase_sector 0 1 3" -c "exit"

dump: # flash read_bank num filename [offset [length]]
	$(OPENOCD) -c "init" -c "halt" -c "flash read_bank 0 dump.bin" -c "exit"

upload:
	$(OPENOCD) -c "program Build/${PROJECT}.elf verify reset exit"

lock:
	$(OPENOCD) -c "init" -c "halt" -c "flash protect 0 0 1 on" -c "flash info 0" -c "exit"

deploy: upload lock

sysex:
	FirmwareSender -in "Build/${PROJECT}.bin" -flash `crc32 "Build/${PROJECT}.bin"` -save "Build/${PROJECT}.syx"
