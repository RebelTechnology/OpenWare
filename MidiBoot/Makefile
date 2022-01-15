BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = owl2-boot.ld
include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(wildcard Src/*.c)
CPP_SRC = $(wildcard Src/*.cpp)

# C_SRC += usbd_audio.c
C_SRC += $(OPENWARE)/Source/usbd_midi.c
C_SRC += $(OPENWARE)/Source/sdram.c
C_SRC += $(OPENWARE)/Source/sysex.c
C_SRC += $(OPENWARE)/Source/crc32.c
C_SRC += $(OPENWARE)/Source/eepromcontrol.c
C_SRC += $(OPENWARE)/Source/message.c
C_SRC += $(OPENWARE)/Source/Flash_S25FL.c
CPP_SRC += $(OPENWARE)/Source/MidiWriter.cpp
CPP_SRC += $(OPENWARE)/Source/VersionToken.cpp
CPP_SRC += $(OPENWARE)/Source/Storage.cpp
# S_SRC = startup_openboot.s

ifndef PLATFORM
PLATFORM = Alchemist
endif

PROJECT = MidiBoot-$(PLATFORM)

C_SRC += usbd_desc.c usbd_conf.c

ifeq ($(PLATFORM),Magus)
  CPPFLAGS += -Iusbd-fs -DOWL_MAGUS -DUSE_USBD_FS
  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Lich)
  CPPFLAGS += -Iusbd-fs -DOWL_LICH -DUSE_USBD_FS
  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Witch)
  CPPFLAGS += -Iusbd-fs -DOWL_WITCH -DUSE_USBD_FS
  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Alchemist)
  CPPFLAGS += -Iusbd-hs -DOWL_ALCHEMIST -DUSE_USBD_HS
  vpath %.c usbd-hs
else ifeq ($(PLATFORM),Wizard)
  CPPFLAGS += -Iusbd-hs -DOWL_WIZARD -DUSE_USBD_HS
  vpath %.c usbd-hs
else ifeq ($(PLATFORM),Pedal)
  CPPFLAGS += -Iusbd-hs -DOWL_PEDAL -DUSE_USBD_HS
  vpath %.c usbd-hs
else ifeq ($(PLATFORM),EuroWizard)
  CPPFLAGS += -Iusbd-fs -DOWL_WIZARD -DUSE_USBD_FS
  vpath %.c usbd-fs
else ifeq ($(PLATFORM),Rack)
  CPPFLAGS += -Iusbd-hs -DOWL_RACK -DUSE_USBD_HS
  vpath %.c usbd-hs
else ifeq ($(PLATFORM),Noctua)
  CPPFLAGS += -Iusbd-fs -DOWL_NOCTUA -DUSE_USBD_FS
  vpath %.c usbd-fs
else ifeq ($(PLATFORM),BioSignals)
  CPPFLAGS += -Iusbd-hs -DOWL_BIOSIGNALS -DUSE_USBD_HS
  vpath %.c usbd-hs
else
  $(error Invalid PLATFORM=$(PLATFORM))
endif

CPP_SRC += $(OPENWARE)/Source/MidiController.cpp

C_SRC += $(C_SRC_SDRAM)
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
	$(OPENOCD) -c "init" -c "halt" -c "flash erase_sector 0 0 3" -c "exit"

erase-storage: # flash erase_sector num first last
	$(OPENOCD) -c "init" -c "halt" -c "flash erase_sector 0 7 11" -c "exit"

dump: # flash read_bank num filename [offset [length]]
	$(OPENOCD) -c "init" -c "halt" -c "flash read_bank 0 dump.bin" -c "exit"

upload:
	$(OPENOCD) -c "program Build/${PROJECT}.elf verify reset exit"

lock:
	$(OPENOCD) -c "init" -c "halt" -c "flash protect 0 0 3 on" -c "flash info 0" -c "exit"

deploy: upload lock

sysex:
	FirmwareSender -in "Build/${PROJECT}.bin" -flash `crc32 "Build/${PROJECT}.bin"` -save "Build/${PROJECT}.syx"

