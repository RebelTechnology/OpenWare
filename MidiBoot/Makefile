PROJECT = MidiBoot
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

LDSCRIPT = STM32F427ZI-boot.ld
include $(OPENWARE)/Hardware/owl2.mk

C_SRC = $(notdir $(wildcard Src/*.c))
CPP_SRC = $(notdir $(wildcard Src/*.cpp))

C_SRC += usbd_midi.c
C_SRC += usbd_midi_if.c
C_SRC += sdram.c
C_SRC += sysex.c
C_SRC += crc32.c
C_SRC += eepromcontrol.c
# S_SRC = startup_openboot.s

# CPP_SRC += MidiReader.cpp
CPP_SRC += MidiController.cpp

OBJS = $(C_SRC:%.c=Build/%.o)
OBJS += $(CPP_SRC:%.cpp=Build/%.o)
OBJS += $(S_SRC:%.s=Build/%.o)

OBJS += $(OBJS_SDRAM)
OBJS += $(OBJS_USBD)
OBJS += $(OBJS_UART)

include $(OPENWARE)/Hardware/f4.mk

.PHONY: clean deploy unlock erase upload lock

unlock:
	openocd -f openocd.cfg -c "init" -c "halt" -c "flash protect 0 0 11 off" -c "flash info 0" -c "exit"

erase:
	openocd -f openocd.cfg -c "init" -c "halt" -c "stm32f2x mass_erase 0" -c "flash info 0" -c "exit"

upload:
	openocd -f openocd.cfg -c "program Build/MidiBoot.elf verify reset exit"

lock:
	openocd -f openocd.cfg -c "init" -c "halt" -c "flash protect 0 0 1 on" -c "flash info 0" -c "exit"

deploy: upload lock
