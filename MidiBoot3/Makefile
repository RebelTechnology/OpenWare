BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = owl3-boot.ld
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
C_SRC += $(OPENWARE)/Source/support.c
C_SRC += $(OPENWARE)/Source/flash-spi.c
C_SRC += $(OPENWARE)/Source/flash-qspi.c
CPP_SRC += $(OPENWARE)/Source/MidiWriter.cpp
CPP_SRC += $(OPENWARE)/Source/VersionToken.cpp
CPP_SRC += $(OPENWARE)/Source/Storage.cpp
CPP_SRC += $(OPENWARE)/Source/MidiBoot.cpp

ifndef PLATFORM
PLATFORM = Genius
endif

PROJECT = MidiBoot-$(PLATFORM)

ifeq ($(PLATFORM),Genius)
  CPPFLAGS += -Iusbd-fs -DOWL_GENIUS -DUSE_USBD_FS
  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Magus)
  CPPFLAGS += -Iusbd-fs -DOWL_MAGUS -DUSE_USBD_FS
  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Xibeca)
  CPPFLAGS += -Iusbd-fs -DOWL_XIBECA -DUSE_USBD_FS
  vpath %.c usbd-fs
else
  $(error Invalid PLATFORM=$(PLATFORM))
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
	$(OPENOCD) -c "init" -c "halt" -c "stm32h7x mass_erase 0" -c "flash info 0" -c "exit"

erase-boot: # flash erase_sector num first last
	$(OPENOCD) -c "init" -c "halt" -c "flash erase_sector 0 0 1" -c "exit"

erase-storage: # flash erase_sector bank first last
	$(OPENOCD) -c "init" -c "halt" -c "flash erase_sector 1 0 3" -c "exit" # Genius 1M part has dual 512K banks

dump: # flash read_bank num filename [offset [length]]
	$(OPENOCD) -c "init" -c "halt" -c "flash read_bank 0 dump.bin" -c "exit"

upload:
	$(OPENOCD) -c "program Build/${PROJECT}.elf verify reset exit"

lock:
	$(OPENOCD) -c "init" -c "halt" -c "flash protect 0 0 1 on" -c "flash info 0" -c "exit"

deploy: upload lock

sysex:
	FirmwareSender -in "Build/${PROJECT}.bin" -flash `crc32 "Build/${PROJECT}.bin"` -save "Build/${PROJECT}.syx"

dfu:
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000 -D Build/${PROJECT}.elf
	@echo Uploaded $< to device bootsector
