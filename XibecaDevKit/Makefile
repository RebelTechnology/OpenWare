PROJECT = XibecaDevKit
BUILDROOT = .
OPENWARE ?= $(BUILDROOT)/..

ifndef PLATFORM
PLATFORM = Red
endif

ifeq ($(PLATFORM),Green)
  CPPFLAGS += -DUSE_FFT_TABLES
  LDSCRIPT = $(OPENWARE)/Hardware/xibeca-big.ld
else ifeq ($(PLATFORM),Red)
  CPPFLAGS += -DUSE_FAST_POW_RESOURCES
  LDSCRIPT = $(OPENWARE)/Hardware/xibeca.ld
else
  $(error Invalid PLATFORM=$(PLATFORM))
endif

include $(OPENWARE)/Hardware/xibeca.mk

C_SRC = $(wildcard Core/Src/*.c)
CPP_SRC = $(wildcard Core/Src/*.cpp)
C_SRC += $(OPENWARE)/Source/pcm3168a.c
C_SRC += $(OPENWARE)/Source/sdram-as4cXm32.c

C_SRC += $(OPENWARE)/Source/flash-qspi.c

CPP_SRC += $(OPENWARE)/Source/uart.cpp
CPP_SRC += $(OPENWARE)/Source/uart_midi.cpp
CPP_SRC += $(OPENWARE)/Source/MidiStreamReader.cpp

include $(OPENWARE)/Hardware/sources.mk

C_SRC += $(C_SRC_SAI)
C_SRC += $(C_SRC_DAC)
C_SRC += $(C_SRC_SDRAM)
C_SRC += $(C_SRC_DSP)
C_SRC += $(C_SRC_OS)
C_SRC += $(C_SRC_UART)
C_SRC += $(C_SRC_I2C)
C_SRC += $(C_SRC_QSPI)
C_SRC += $(C_SRC_MDMA)
C_SRC += $(C_SRC_USBD)
C_SRC += $(C_SRC_USBD_AUDIO)
C_SRC += $(C_SRC_USBH)
C_SRC += $(C_SRC_USBH_MIDI)

include $(OPENWARE)/Hardware/h7.mk

.PHONY: dfu fastpow fastlog

dfu: Build/${PROJECT}.bin
	$(DFUUTIL) -d 0483:df11 -c 1 -i 0 -a 0 -s 0x8000000 -D $<
	@echo Uploaded $< to Xibeca bootsector

Build/fastpow.bin: ../Source/fastpow.cpp
	arm-none-eabi-gcc -c ../Source/fastpow.cpp -o Build/fastpow.o
	arm-none-eabi-objcopy -O binary Build/fastpow.o
	dd if=Build/fastpow.o bs=4 skip=2 of=Build/fastpow.bin

Build/fastlog.bin: ../Source/fastlog.cpp
	arm-none-eabi-gcc -c ../Source/fastlog.cpp -o Build/fastlog.o
	arm-none-eabi-objcopy -O binary Build/fastlog.o
	dd if=Build/fastlog.o bs=4 skip=2 of=Build/fastlog.bin

fastpow: Build Build/fastpow.bin
	FirmwareSender -out "OWL*" -in Build/fastpow.bin -name SPW.bin

fastlog: Build Build/fastlog.bin
	FirmwareSender -out "OWL*" -in Build/fastlog.bin -name SLG.bin
